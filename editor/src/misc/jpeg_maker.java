package misc;

import com.sun.image.codec.jpeg.*;
import java.awt.*;
import java.awt.image.*;
import java.io.*;

/** This class makes a jpeg file from a string filename and a image */

public class jpeg_maker {
    public jpeg_maker(String file_name, BufferedImage Image) throws Exception {
	try{
	    BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(file_name));
	    JPEGImageEncoder encoder = JPEGCodec.createJPEGEncoder(out);
	    JPEGEncodeParam param = encoder.getDefaultJPEGEncodeParam(Image);
	    int quality = 100;
	    quality = Math.max(0, Math.min(quality, 100));
	    param.setQuality((float)quality / 100.0f, false);
	    encoder.setJPEGEncodeParam(param);
	    encoder.encode(Image);
	}catch(Exception e){
	    System.err.println("Jpeg writing failed : "+file_name);
	}
    }
}
