#include <stdio.h>
#include <string.h>
#include <iconv.h>

int main( int argc, char * argv[] )
{
        // EUC-KR 문자열을 UTF8 문자열로 변환할 수 있도록 호출한다.
        iconv_t iConv = iconv_open( "UTF-8", "EUC-KR" );

        // EUC-KR 로 "가" 문자열을 저장한다.
        char szInput[] = "1";
        char szOutput[100];
        size_t iInLen = strlen((char*)szInput);
        size_t iOutLen = sizeof(szOutput);
        char * pszInput = (char *)szInput;
        char * pszOutput = szOutput;

        memset( szOutput, 0, sizeof(szOutput) );

        // EUC-KR 문자열을 UTF8 문자열로 변환한다.
        int n = iconv( iConv, &pszInput, &iInLen, &pszOutput, &iOutLen );

        // 변환 결과를 출력한다.
        char a[] = "1";
        if(a[0] == szInput[0]){
                printf("success\n");
        }

        iconv_close( iConv );
        return 0;
}
