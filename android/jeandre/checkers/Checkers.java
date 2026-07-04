/*
 * Copyright (c) 2026 Jeandre Kruger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package jeandre.checkers;

import java.io.*;

class Checkers {
	private MainActivity activity;

	public native void init(int width, int height);
	public native void update();
	public native void inputEvent(int button);
	public native void mouseMoveEvent(int x, int y);
	public native void mouseUpEvent(int x, int y);

	private native void textInputAccept(String value);
	private native void textInputCancel();

	public Checkers(MainActivity activity) {
		this.activity = activity;
	}

	public byte[] textureDataFromAsset(String fileName)
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

	public void getTextInput(String label) {
		activity.getTextInput(label, this::textInputAccept, this::textInputCancel);
	}

	static {
		System.loadLibrary("checkers");
	}
}
