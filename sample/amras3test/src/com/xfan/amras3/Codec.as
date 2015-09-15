package com.xfan.amras3
{
	import com.xfan.amras3.flascc.*;
	
	import com.xfan.amras3.flascc.CModule;
	import com.xfan.amras3.flascc.ram;
	
	import flash.utils.ByteArray;
	
	public class Codec
	{
		static public function encode(src:ByteArray):ByteArray
		{
			var dest:ByteArray = new ByteArray;
			
			amras3_encode(src, dest);
			
			return dest;
		}
	}
}