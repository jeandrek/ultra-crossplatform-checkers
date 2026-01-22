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

import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.*;
import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.*;
import android.widget.*;

public class MainActivity extends Activity {
	private static final int buttonIds[] = {
		R.id.upButton,
		R.id.downButton,
		R.id.leftButton,
		R.id.rightButton,
		R.id.pauseButton,
		R.id.acceptButton,
		R.id.backButton
	};

	private Checkers checkers;
	private GLSurfaceView view;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		checkers = new Checkers(this);

		setContentView(R.layout.main);

		view = new GLSurfaceView(getApplicationContext());
		view.setRenderer(new GLSurfaceView.Renderer() {
			@Override
			public void onSurfaceCreated(GL10 gl, EGLConfig config) {
				checkers.init(view.getWidth(),
					      view.getHeight());
			}

			@Override
			public void onSurfaceChanged(GL10 gl, int width, int height) {
				//gl.glViewport(0, 0, width, height);
			}

			@Override
			public void onDrawFrame(GL10 gl) {
				checkers.update();
			}
		});
		view.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
		((RelativeLayout)findViewById(R.id.main)).addView(view, 0);
		for (int i = 0; i < buttonIds.length; i++)
			((Button)findViewById(buttonIds[i]))
				.setOnClickListener(onClickFor(i));
	}

	private View.OnClickListener onClickFor(int button) {
		return new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				checkers.inputEvent(button);
			}
		};
	}

	@Override
	protected void onStart() {
		super.onStart();
		view.onResume();
	}

	@Override
	protected void onStop() {
		super.onStop();
		view.onPause();
	}
}
