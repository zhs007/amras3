package
{
	import com.xfan.amras3.Codec;
	
	import flash.display.Sprite;
	import flash.utils.ByteArray;
	
	public class amras3test extends Sprite
	{
		public function amras3test()
		{
			var buff:ByteArray = new ByteArray;
			
			buff.writeByte(0);
			buff.writeByte(7);
			buff.writeByte(2);
			buff.writeByte(9);
			buff.writeByte(0);
			
			buff.position = 0;
			
			var buff2:ByteArray = Codec.encode(buff);
			
			trace("len is " + buff2.length);
			
			buff2.position = 0;
			buff.position = 0;
			for (var i:int = 0; i < buff2.length; ++i)
			{
				var val:int = buff2.readByte();
				trace("cur " + i + " is " + val);
				//var val1:int = buff.readByte();
				//trace("cur1 " + i + " is " + val1);
			}
		}
	}
}