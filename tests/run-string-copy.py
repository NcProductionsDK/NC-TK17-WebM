"""Compare production string copying with its original Windows implementation."""
import os
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parent.parent
source = (root / 'NC-TK17-WebM.c').read_text(encoding='utf-8')

def function(name):
    match = re.search(r'^static int ' + name + r'\([^;{}]*\)\s*\{', source, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[match.start():end] + '\n'

fixture = r'''
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
'''
fixture += function('ptr_readable') + function('copy_engine_string_a')
fixture += r'''
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
'''
build = root / 'build/string-copy-tests'
build.mkdir(parents=True, exist_ok=True)
cfile, exe = build / 'string_copy.c', build / 'string_copy.exe'
cfile.write_text(fixture, encoding='utf-8')
env = dict(os.environ)
env['PATH'] = r'C:\msys64\mingw32\bin;' + env.get('PATH', '')
subprocess.run([r'C:\msys64\mingw32\bin\gcc.exe', '-m32', '-O2', '-Wall',
                '-Wextra', '-Werror', str(cfile), '-o', str(exe)], env=env, check=True)
subprocess.run([str(exe)], env=env, check=True)
