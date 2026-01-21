package jeandre.checkers;

import java.io.*;
import android.app.Activity;

class Checkers {
	private static Activity activity;

	public static native void init(int width, int height);
	public static native void update();
	public static native void inputEvent(int button);

	public static void setActivity(Activity newActivity) {
		activity = newActivity;
	}

	private static byte[] textureDataFromAsset(String fileName) throws IOException {
		InputStream stream = activity.getAssets().open(fileName);
		stream.mark(Integer.MAX_VALUE);
		int length = (int)stream.skip(1 << 20);
		byte[] pixels = new byte[length];
		stream.reset();
		stream.read(pixels, 0, length);
		stream.close();
		return pixels;
	}

	static {
		System.loadLibrary("checkers");
	}
}
