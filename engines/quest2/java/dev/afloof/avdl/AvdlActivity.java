package dev.afloof.avdl;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.opengl.GLES20;
import android.os.Bundle;
import android.view.MotionEvent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaPlayer;

import java.io.InputStream;
import android.content.res.Resources;
import java.io.BufferedReader;
import android.util.Log;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.*;

// open url
import android.content.Intent;
import android.net.Uri;

import android.content.res.AssetManager;

public class AvdlActivity extends android.app.NativeActivity {
	public static AvdlActivity activity;
	public static Context context;

	public static final int AUDIO_MAX = 10;
	public static MediaPlayer mediaPlayer[] = new MediaPlayer[AUDIO_MAX];
	public static int mediaPlayerId[] = new int[AUDIO_MAX];

	private AssetManager assetManager;

	static {
		System.loadLibrary("ovrplatformloader");
		System.loadLibrary("openxr_loader");
		System.loadLibrary("avdlproject");
	}

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		AvdlActivity.activity = this;
		AvdlActivity.context = getApplicationContext();
		assetManager = getResources().getAssets();
		nativeSetAssetManager(assetManager);
	}

	static Object[] ReadBitmap(String bitmapName) {
		int id = AvdlActivity.context.getApplicationContext().getResources().getIdentifier(
			bitmapName, "drawable", AvdlActivity.context.getPackageName()
		);
		//Log.w("avdl", "id: " +id);
		if (id == 0) {
			return null;
		}

		Bitmap b = BitmapFactory.decodeResource(AvdlActivity.context.getApplicationContext().getResources(), id);

		int arraySize[] = new int[] {
			b.getWidth(),
			b.getHeight(),
		};
		int[] array1 = new int[b.getWidth() *b.getHeight()];
		b.getPixels(array1, 0, b.getWidth(), 0, 0, b.getWidth(), b.getHeight());

		return new Object[] {arraySize, array1};
	}

	static int OpenUrl(String url) {
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		AvdlActivity.activity.startActivity(browserIntent);
		return 1;
	}

	static int PlayAudio(String resourceName, int loop, int avdl_id) {

		int id = AvdlActivity.context.getApplicationContext().getResources().getIdentifier(
			resourceName, "raw", AvdlActivity.context.getPackageName()
		);
		if (id == 0) {
			Log.w("avdl", "no resource id for " +resourceName);
			return -1;
		}

		// find index for this audio
		int desiredIndex = -1;
		for (int i = 0; i < AUDIO_MAX; i++) {
			if (mediaPlayer[i] == null) {
				desiredIndex = i;
				break;
			}
		}

		// no index available - too many sounds playing
		if (desiredIndex == -1) {
			Log.w("avdl", "warning: too many sounds playing - skipping audio");
			return -1;
		}

		MediaPlayer mp = MediaPlayer.create(AvdlActivity.context, id);
		if (loop == 1) {
			mp.setLooping(true);
		}
		mediaPlayer[desiredIndex] = mp;
		mediaPlayerId[desiredIndex] = avdl_id;
		mp.start();

		// on complete, remove media player
		mp.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
			public void onCompletion(MediaPlayer mp) {
				mp.release();
				for (int i = 0; i < AUDIO_MAX; i++) {
					if (mediaPlayer[i] == mp) {
						mediaPlayer[i] = null;
						mediaPlayerId[i] = 0;
					}
				}
			}
		});

		return desiredIndex;

	}

	static void StopAudio(int avdl_id) {
		for (int i = 0; i < AUDIO_MAX; i++) {
			if (mediaPlayerId[i] == avdl_id) {
				mediaPlayer[i].stop();
				mediaPlayer[i].release();
				mediaPlayer[i] = null;
			}
		}
	}

	private static native void nativeSetAssetManager(AssetManager assetManager);

}
