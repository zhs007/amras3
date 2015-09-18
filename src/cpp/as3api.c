#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AS3/AS3.h"

#include "amrFileCodec.h"
#include "codec/sp_enc.h"

////** amras3_encodeex **////
void amras3_encodeex() __attribute__((used,
     annotate("as3sig:public function amras3_encodeex(srcBuff:int, srcLen:int, destBuff:int):int"),
     annotate("as3package:com.xfan.amras3.flascc")));

void amras3_encodeex()
{
	//inline_as3("trace(\"ver is 52\");");
	
	char *byteArray_src;
	char *byteArray_dest;
	char *buff;
    int len;
	int destlen;
	int i;
	
    // convert arguments
    AS3_GetScalarFromVar(byteArray_src, srcBuff);
    AS3_GetScalarFromVar(len, srcLen);
	AS3_GetScalarFromVar(byteArray_dest, destBuff);
	
	buff = EncodeAMR(byteArray_src, len, 1, 16, &destlen);

	inline_as3("trace(\"destlen is \" + %0);" : : "r"(destlen));
	inline_as3("trace(\"len is \" + %0);" : : "r"(len));
	
	if (destlen > len) {
		inline_as3("trace(\"dest len fail!\");");
	}
	else {
		memcpy(byteArray_dest, buff, destlen);
		free(buff);
	}

	inline_as3("trace(\"src len is \" + %0);" : : "r"(len));
	inline_as3("trace(\"dest len is \" + %0);" : : "r"(destlen));
	
	inline_as3("var retval:int = %0;" : : "r"(destlen));

	AS3_ReturnAS3Var(retval);
}

////** amras3_decodeex **////
void amras3_decodeex() __attribute__((used,
     annotate("as3sig:public function amras3_decodeex(srcBuff:int, srcLen:int, retBegin:int):void"),
     annotate("as3package:com.xfan.amras3.flascc")));

void amras3_decodeex()
{
	//inline_as3("trace(\"ver is 55\");");
	
	char *byteArray_src;
	int* byteArray_ret;
	char *buff;
    int len;
	int destlen;
	int i;
	
    // convert arguments
    AS3_GetScalarFromVar(byteArray_src, srcBuff);
    AS3_GetScalarFromVar(len, srcLen);
	AS3_GetScalarFromVar(byteArray_ret, retBegin);
	
	byteArray_ret[0] = (int)DecodeAMR(byteArray_src, len, &destlen);
	byteArray_ret[1] = destlen;

	inline_as3("trace(\"destbuf is \" + %0);" : : "r"(byteArray_ret[0]));
	inline_as3("trace(\"destlen is \" + %0);" : : "r"(destlen));
	inline_as3("trace(\"len is \" + %0);" : : "r"(len));
	
    // return void
    AS3_ReturnAS3Var(undefined);
}
