// Copyright (c) 2013 Adobe Systems Inc

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AS3/AS3.h"

////** amras3_encode **////
void amras3_encode() __attribute__((used,
     annotate("as3sig:public function amras3_encode(srcBuff:ByteArray, destBuff:ByteArray):void"),
     annotate("as3package:com.xfan.amras3.flascc"),
	 annotate("as3import:flash.utils.ByteArray")));

void amras3_encode()
{
	inline_as3("trace(\"ver is 30\");");
	
	char *byteArray_c;
	char *byteArray_c1;
    unsigned int len;

    inline_as3("%0 = srcBuff.bytesAvailable;" : "=r"(len));
    byteArray_c = (char *)malloc(len);
	byteArray_c1 = (char *)malloc(len);

    inline_as3("CModule.ram.position = %0;" : : "r"(byteArray_c));
    inline_as3("srcBuff.readBytes(CModule.ram);");	
	
	//AS3_Trace(len);
	//printf("len is %d", len);
	inline_as3("trace(\"byteArray_c is \" + %0);" : : "r"(byteArray_c));
	inline_as3("trace(\"len is \" + %0);" : : "r"(len));
	int val;
	for (int i = 0; i < len; ++i) {
		//inline_as3("trace(\"src0 is \" + %0);" : : "r"(i));
		//val = byteArray_c[i];
		inline_as3("trace(\"src11 is \" + %0);" : : "r"(byteArray_c[i]));
		//byteArray_c1[i] = byteArray_c[i] + 1;
		//val = byteArray_c1[i];
		//inline_as3("trace(\"src21 is \" + %0);" : : "r"(byteArray_c1[i]));
		
		//inline_as3("destBuff.writeByte(%0);" : : "r"(val));
	}
	
	inline_as3("destBuff.position = 0;");
	//inline_as3("CModule.ram.position = %0;" : : "r"(byteArray_c));
	inline_as3("destBuff.writeBytes(CModule.ram, %0, %1);" : : "r"(byteArray_c1), "r"(len));
	
	//free(byteArray_c);
    // return void
	AS3_ReturnAS3Var(undefined);
	
	//inline_as3("var retval = %0;" : : "r"(byteArray_c1));
	
    //AS3_ReturnAS3Var(retval);
}

////** amras3_free **////
void amras3_free() __attribute__((used,
     annotate("as3sig:public function amras3_free(buff:int):void"),
     annotate("as3package:com.xfan.amras3.flascc")));

void amras3_free()
{
    unsigned char *byteArray_c = NULL;
	
    // convert arguments
    AS3_GetScalarFromVar(byteArray_c, buff);
	
	free(byteArray_c);
    // return void
    AS3_ReturnAS3Var(undefined);
}