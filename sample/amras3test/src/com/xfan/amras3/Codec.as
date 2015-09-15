package com.xfan.amras3
{
	import com.xfan.amras3.flascc.CModule;
	import com.xfan.amras3.flascc.amras3_encode;
	import com.xfan.amras3.flascc.ram;
	
	import flash.utils.ByteArray;
	
	public class Codec
	{
		static public function encode(src:ByteArray):ByteArray
		{
			var buff:ByteArray = new ByteArray;
			amras3_encode(src, buff);
			//var buff:ByteArray = new ByteArray;
			//trace(dest);
			//ram.position = dest;
			//buff.writeBytes(ram, dest, 5);
			
			return buff;
		}
	}
}