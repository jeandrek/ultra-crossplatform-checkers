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
import java.util.function.Consumer;
import android.app.*;
import android.content.DialogInterface;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.*;
import android.widget.*;
import android.R;

public class MainActivity extends Activity {
	private Checkers checkers;
	private GLSurfaceView view;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		checkers = new Checkers(this);

		view = new GLSurfaceView(getApplicationContext());
		view.setOnTouchListener(new View.OnTouchListener() {
			@Override
			public boolean onTouch(View view, MotionEvent event) {
				switch (event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					checkers.mouseMoveEvent((int)event.getX(),
								(int)event.getY());
					return true;
				case MotionEvent.ACTION_MOVE:
					checkers.mouseMoveEvent((int)event.getX(),
								(int)event.getY());
					return true;
				case MotionEvent.ACTION_UP:
					checkers.mouseUpEvent((int)event.getX(),
							      (int)event.getY());
					return true;
				}
				return false;
			}
		});
		view.setRenderer(new GLSurfaceView.Renderer() {
			@Override
			public void onSurfaceCreated(GL10 gl, EGLConfig config) {
				checkers.init(view.getWidth(),
					      view.getHeight());
			}

			@Override
			public void onSurfaceChanged(GL10 gl, int width, int height) {
				checkers.resize(width, height);
			}

			@Override
			public void onDrawFrame(GL10 gl) {
				checkers.update();
			}
		});
		view.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
		setContentView(view);
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

	/*
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
	}
	*/

	public static class TextInputDialogFragment extends DialogFragment {
		private String label;
		private Consumer<String> accept;
		private Runnable cancel;
		private EditText input;
		public void initTextInput(String label,
					  Consumer<String> accept,
					  Runnable cancel) {
			this.label = label;
			this.accept = accept;
			this.cancel = cancel;
		}
		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState) {
			AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
			input = new EditText(getActivity().getApplicationContext());
			DialogInterface.OnClickListener listener =
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						if (which == DialogInterface.BUTTON_POSITIVE) {
							accept.accept(input.getText().toString());
						} else {
							cancel.run();
						}
					}
				};
			builder.setTitle(label);
			builder.setPositiveButton(R.string.ok, listener);
			builder.setNegativeButton(R.string.cancel, listener);
			builder.setView(input);
			return builder.create();
		}
	}

	public void getTextInput(String label, Consumer<String> accept, Runnable cancel) {
		TextInputDialogFragment fragment = new TextInputDialogFragment();
		fragment.initTextInput(label, accept, cancel);
		fragment.show(getFragmentManager(), "TEXT_INPUT_DIALOG");
	}
}
