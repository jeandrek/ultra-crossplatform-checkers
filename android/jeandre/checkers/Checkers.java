package jeandre.checkers;

import java.io.*;
import android.app.Activity;

class Checkers {
	private Activity activity;

	public native void init(int width, int height);
	public native void update();
	public native void inputEvent(int button);

	public Checkers(Activity activity) {
		this.activity = activity;
	}

	private byte[] textureDataFromAsset(String fileName)
		throws IOException {
		InputStream stream = activity.getAssets().open(fileName);
		stream.mark(Integer.MAX_VALUE);
		int length = (int)stream.skip(1 << 20);
		stream.reset();
		byte[] pixels = new byte[length];
		stream.read(pixels, 0, length);
		stream.close();
		return pixels;
	}

	static {
		System.loadLibrary("checkers");
	}
}
