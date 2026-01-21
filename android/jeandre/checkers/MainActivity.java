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

	private GLSurfaceView view;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Checkers.setActivity(this);

		setContentView(R.layout.main);

		view = new GLSurfaceView(getApplicationContext());
		view.setRenderer(new GLSurfaceView.Renderer() {
			@Override
			public void onSurfaceCreated(GL10 gl, EGLConfig config) {
				Checkers.init(view.getWidth(),
					      view.getHeight());
			}

			@Override
			public void onSurfaceChanged(GL10 gl, int width, int height) {
				//gl.glViewport(0, 0, width, height);
			}

			@Override
			public void onDrawFrame(GL10 gl) {
				Checkers.update();
			}
		});
		view.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
		((RelativeLayout)findViewById(R.id.main)).addView(view, 0);
		for (int i = 0; i < buttonIds.length; i++)
			((Button)findViewById(buttonIds[i]))
				.setOnClickListener(onClickFor(i));
	}

	private View.OnClickListener onClickFor(int button) {
		MainActivity activity = this;
		return new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Checkers.inputEvent(button);
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
