
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
static unsigned queries;
static SIZE_T counted_query(LPCVOID p, PMEMORY_BASIC_INFORMATION m, SIZE_T n) {
    queries++; return VirtualQuery(p,m,n);
}
#define VirtualQuery counted_query
static int ptr_readable(const void *p, size_t bytes)
{
    MEMORY_BASIC_INFORMATION mbi;
    DWORD protect;
    if (!p || !VirtualQuery(p, &mbi, sizeof(mbi))) return 0;
    protect = mbi.Protect & 0xff;
    if (mbi.State != MEM_COMMIT) return 0;
    if (protect == PAGE_NOACCESS || protect == PAGE_EXECUTE || (mbi.Protect & PAGE_GUARD)) return 0;
    return (const BYTE*)p + bytes <= (const BYTE*)mbi.BaseAddress + mbi.RegionSize;
}
static int copy_engine_string_a(const char *value, char *out, size_t outsz)
{
    int length;
    size_t i;
    if (!out || !outsz) return 0;
    out[0] = 0;
    if (!value || !ptr_readable(value, 1)) return 0;
    if (ptr_readable(value - sizeof(int), sizeof(int))) {
        memcpy(&length, value - sizeof(int), sizeof(length));
        if (length >= 0 && length < 32768 && ptr_readable(value, (size_t)length + 1)) {
            size_t copy_length = (size_t)length;
            if (copy_length >= outsz) copy_length = outsz - 1;
            memcpy(out, value, copy_length);
            out[copy_length] = 0;
            return 1;
        }
    }
    /* Plain C-string fallback: validate each region once, rather than
       querying Windows for every character. Keep permissions local to this
       copy and recheck before crossing a region boundary. */
    i = 0;
    while (i + 1 < outsz) {
        MEMORY_BASIC_INFORMATION mbi;
        uintptr_t address = (uintptr_t)value + i;
        uintptr_t region_end;
        size_t count;
        DWORD protect;
        if (address < (uintptr_t)value ||
            !VirtualQuery((const void*)address, &mbi, sizeof(mbi))) return 0;
        protect = mbi.Protect & 0xff;
        if (mbi.State != MEM_COMMIT || protect == PAGE_NOACCESS ||
            protect == PAGE_EXECUTE || (mbi.Protect & PAGE_GUARD)) return 0;
        region_end = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
        if (region_end <= address) return 0;
        count = region_end - address;
        if (count > outsz - 1 - i) count = outsz - 1 - i;
        while (count--) {
            out[i] = value[i];
            if (!out[i]) return 1;
            i++;
        }
    }
    out[outsz - 1] = 0;
    return 1;
}

static int reference(const char *value,char *out,size_t outsz) {
    int length; size_t i;
    if (!out || !outsz) return 0;
    out[0]=0;
    if (!value || !ptr_readable(value,1)) return 0;
    if (ptr_readable(value-sizeof(int),sizeof(int))) {
        memcpy(&length,value-sizeof(int),sizeof(length));
        if (length>=0 && length<32768 && ptr_readable(value,(size_t)length+1)) {
            size_t n=(size_t)length;
            if (n>=outsz) n=outsz-1;
            memcpy(out,value,n); out[n]=0; return 1;
        }
    }
    for (i=0;i+1<outsz;i++) {
        if (!ptr_readable(value+i,1)) return 0;
        out[i]=value[i]; if (!out[i]) return 1;
    }
    out[outsz-1]=0; return 1;
}
static void compare(const char *value,size_t size) {
    char expected[512],actual[512]; assert(size<=sizeof(actual));
    memset(expected,0x5a,sizeof(expected)); memset(actual,0x5a,sizeof(actual));
    int a=reference(value,expected,size);
    int b=copy_engine_string_a(value,actual,size);
    assert(a==b); assert(!memcmp(expected,actual,sizeof(actual)));
}
static double timer(void) {
    LARGE_INTEGER t,f; QueryPerformanceCounter(&t); QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/f.QuadPart;
}
int main(void) {
    SYSTEM_INFO si; GetSystemInfo(&si); size_t page=si.dwPageSize;
    char *p=VirtualAlloc(NULL,page*3,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    assert(p); DWORD old; char out[512];
    memset(p,0xff,page*3); char *s=p+64;
    assert(!copy_engine_string_a(s,NULL,1)); compare(NULL,0); compare(NULL,10);
    compare((const char*)1,4);
    /* Invalid engine-length prefix forces the fallback. Test arbitrary bytes,
       embedded NULs and all small output sizes, including zero and one. */
    for (unsigned byte=0;byte<256;byte++) {
        memset(s,'a',255); s[17]=(char)byte; s[255]=0;
        for (size_t size=0;size<=260;size+=5) compare(s,size);
        compare(s,1); compare(s,2);
    }
    /* Length-based behavior (including embedded NUL and absent terminator)
       is preserved; prefix values outside the range select the fallback. */
    int lengths[]={0,1,20,255,32767,32768,-1};
    for (unsigned j=0;j<sizeof(lengths)/sizeof(lengths[0]);j++) {
        memcpy(s-4,&lengths[j],4); memset(s,'b',255); s[5]=0; s[255]=0;
        for (size_t size=0;size<300;size+=7) compare(s,size);
    }
    puts("PASS: length prefix, embedded NUL, byte values, truncation and output boundaries");
    memset(p,0xff,page*3);
    s=p+page-32; memset(s,'x',64); s[64]=0;
    assert(VirtualProtect(p+page,page,PAGE_READONLY,&old));
    compare(s,128); assert(copy_engine_string_a(s,out,sizeof(out)) && strlen(out)==64);
    assert(VirtualProtect(p+page,page,PAGE_NOACCESS,&old));
    compare(s,32); compare(s,33); compare(s,128);
    assert(!copy_engine_string_a(s,out,sizeof(out)));
    /* A terminator immediately before the inaccessible page must succeed. */
    p[page-1]=0; compare(s,128); assert(copy_engine_string_a(s,out,sizeof(out)));
    p[page-1]='x';
    assert(VirtualProtect(p+page,page,PAGE_READWRITE|PAGE_GUARD,&old));
    compare(s,128);
    MEMORY_BASIC_INFORMATION mbi;
    assert(VirtualQuery(p+page,&mbi,sizeof(mbi)) && (mbi.Protect&PAGE_GUARD));
    assert(VirtualProtect(p+page,page,PAGE_EXECUTE,&old)); compare(s,128);
    assert(VirtualFree(p+page,page,MEM_DECOMMIT)); compare(s,128);
    assert(VirtualAlloc(p+page,page,MEM_COMMIT,PAGE_READWRITE));
    memset(p+page,'x',32); p[page+32]=0; compare(s,128);
    assert(copy_engine_string_a(s,out,sizeof(out)));
    assert(VirtualProtect(p+page,page,PAGE_NOACCESS,&old));
    assert(!copy_engine_string_a(s,out,sizeof(out)));
    puts("PASS: cross-region copy, partial failures, inaccessible/guard/execute-only pages and permission changes");
    s=p+64; memset(s-4,0xff,4); memset(s,'z',255); s[255]=0;
    queries=0; assert(reference(s,out,sizeof(out))); unsigned before=queries;
    queries=0; assert(copy_engine_string_a(s,out,sizeof(out))); unsigned after=queries;
    assert(before==258 && after==3);
    printf("PASS: 255-character fallback: %u queries reduced to %u\n",before,after);
    volatile unsigned sink=0;
    for (int mode=0;mode<2;mode++) {
        double start=timer(); queries=0;
        for (int i=0;i<10000;i++) sink+=mode?copy_engine_string_a(s,out,sizeof(out)):reference(s,out,sizeof(out));
        printf("BENCH %s: %.3f ms, %u queries / 10000 fallback copies\n",
               mode?"fixed":"original",(timer()-start)*1000,queries);
    }
    assert(sink==20000); assert(VirtualFree(p,0,MEM_RELEASE)); return 0;
}
