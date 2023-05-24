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

public class AvdlActivity extends android.app.NativeActivity {
	public static AvdlActivity activity;
	public static Context context;

	/* Mirroring - can be piped together for multiple mirroring */
	static int DD_FILETOMESH_SETTINGS_MIRROR_X = 1;
	static int DD_FILETOMESH_SETTINGS_MIRROR_Y = 2;
	static int DD_FILETOMESH_SETTINGS_MIRROR_Z = 4;

	/* Vertex attributes to parse */
	static int DD_FILETOMESH_SETTINGS_POSITION = 8;
	static int DD_FILETOMESH_SETTINGS_COLOUR = 16;
	static int DD_FILETOMESH_SETTINGS_TEX_COORD = 32;

	public static MediaPlayer mediaPlayer[] = new MediaPlayer[5];

	static {
		System.loadLibrary("ovrplatformloader");
		System.loadLibrary("openxr_loader");
		System.loadLibrary("avdlproject");
	}

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		AvdlActivity.activity = this;
		AvdlActivity.context = getApplicationContext();
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

	static Object[] ReadPly(String assetName, int settings) {
		//assetName = "box";
		//Log.w("avdl", assetName);
		int id = AvdlActivity.context.getApplicationContext().getResources().getIdentifier(assetName, "raw", AvdlActivity.context.getPackageName());
		//Log.w("avdl", "id: " +id);
		if (id == 0) {
			Log.w("avdl", "Error loading asset: " +assetName +" with id: " +id);
			return null;
		}

		float[] posArray2;

		// Temporary way to read assets
		InputStream is = AvdlActivity.context.getApplicationContext().getResources().openRawResource(id);
		BufferedReader reader = new BufferedReader(new InputStreamReader(is));
		String line = null;

		// confirm it's a ply file
		try {
			line = reader.readLine();
		} catch (IOException e){line = null;}
		if (line == null || line.compareTo("ply") != 0) {
			Log.w("avdl", "ReadPly: file '" +assetName +"' is not a ply file");
			return null;
		}

		List<PlyElement> el = new ArrayList<PlyElement>();

		// parse lines
		while (true) {
			try {

			line = reader.readLine();
			if (line == null) {
				break;
			}

			// ignore empty lines
			String[] s2 = line.split(" ");
			if (s2.length == 0) {
				continue;
			}

			// ignore comments
			if (s2[0].compareTo("comment") == 0) {
				continue;
			}

			// found element
			if (s2[0].compareTo("element") == 0) {
				if (s2.length < 3) {
					throw new IOException();
				}
				int size = Integer.parseInt(s2[2]);
				el.add(new PlyElement(s2[1], size));
			}

			// found property
			if (s2[0].compareTo("property") == 0) {
				if (s2.length < 3) {
					throw new IOException();
				}

				PlyElement tmpEl = el.get(el.size()-1);

				// list
				if (s2[1].compareTo("list") == 0) {
					tmpEl.properties.add(new PlyProperty(s2[4], s2[3], true, s2[2]));
				}
				// normal property
				else {
					tmpEl.properties.add(new PlyProperty(s2[2], s2[1], false, null));
				}
			}

			// end of header
			if (s2[0].compareTo("end_header") == 0) {
				break;
			}

			} catch (IOException e) {
				Log.w("avdl", "ReadPly: Error while parsing '" +assetName);
				return null;
			}
		}

		int vertexCountFace = 0;
		int vertexPosCount = 0;
		// for each element
		for (int i = 0; i < el.size(); i++) {

			PlyElement e = el.get(i);

			if (e.name.compareTo("face") == 0) {
				vertexCountFace = e.amount;
				continue;
			}

			if (e.name.compareTo("vertex") == 0) {
				vertexPosCount = e.amount;
			}

		}

		// parsable data - everything else is discarded
		float[] vertexPos = null;
		float[] vertexPosFace = null;
		if ((settings | DD_FILETOMESH_SETTINGS_POSITION) != 0) {
			vertexPos = new float[vertexPosCount *3];
			vertexPosFace = new float[vertexCountFace *3 *3];
		}

		int[] vertexCol = null;
		int[] vertexColFace = null;
		if ((settings | DD_FILETOMESH_SETTINGS_COLOUR) != 0) {
			vertexCol = new int[vertexPosCount *3];
			vertexColFace = new int[vertexCountFace *3 *3];
		}

		float[] vertexTex = null;
		float[] vertexTexFace = null;
		if ((settings | DD_FILETOMESH_SETTINGS_TEX_COORD) != 0) {
			vertexTex = new float[vertexPosCount *2];
			vertexTexFace = new float[vertexCountFace *2 *3];
		}
		// for each element
		for (int i = 0; i < el.size(); i++) {

			PlyElement element = el.get(i);

			try {

			// read all elements
			for (int j = 0; j < element.amount; j++) {

				line = reader.readLine();
				String[] s2 = line.split(" ");

				// for each property
				for (int z = 0; z < element.properties.size(); z++) {
					PlyProperty property = element.properties.get(z);

					// vertex attributes
					if (element.name.compareTo("vertex") == 0) {
						if (vertexPos != null && property.name.compareTo("x") == 0) {
							vertexPos[j*3+0] = Float.parseFloat(s2[z]);
						}
						else
						if (vertexPos != null && property.name.compareTo("y") == 0) {
							vertexPos[j*3+1] = Float.parseFloat(s2[z]);
						}
						else
						if (vertexPos != null && property.name.compareTo("z") == 0) {
							vertexPos[j*3+2] = Float.parseFloat(s2[z]);
						}
						else
						// colours
						if (vertexCol != null && property.name.compareTo("red") == 0) {
							vertexCol[j*3+0] = Integer.parseInt(s2[z]);
						}
						else
						if (vertexCol != null && property.name.compareTo("green") == 0) {
							vertexCol[j*3+1] = Integer.parseInt(s2[z]);
						}
						else
						if (vertexCol != null && property.name.compareTo("blue") == 0) {
							vertexCol[j*3+2] = Integer.parseInt(s2[z]);
						}
						else
						// textures
						if (vertexTex != null && property.name.compareTo("s") == 0) {
							vertexTex[j*2+0] = Float.parseFloat(s2[z]);
						}
						else
						if (vertexTex != null && property.name.compareTo("t") == 0) {
							vertexTex[j*2+1] = Float.parseFloat(s2[z]);
						}
					}
					else
					// faces
					if (element.name.compareTo("face") == 0) {
						if (property.name.compareTo("vertex_indices") == 0) {
							int v1i = Integer.parseInt(s2[1]);
							int v2i = Integer.parseInt(s2[2]);
							int v3i = Integer.parseInt(s2[3]);

							if (vertexPos != null) {
								vertexPosFace[j*9+0] = vertexPos[v1i*3+0];
								vertexPosFace[j*9+1] = vertexPos[v1i*3+1];
								vertexPosFace[j*9+2] = vertexPos[v1i*3+2];

								vertexPosFace[j*9+3] = vertexPos[v2i*3+0];
								vertexPosFace[j*9+4] = vertexPos[v2i*3+1];
								vertexPosFace[j*9+5] = vertexPos[v2i*3+2];

								vertexPosFace[j*9+6] = vertexPos[v3i*3+0];
								vertexPosFace[j*9+7] = vertexPos[v3i*3+1];
								vertexPosFace[j*9+8] = vertexPos[v3i*3+2];
							}

							if (vertexCol != null) {
								vertexColFace[j*9+0] = vertexCol[v1i*3+0];
								vertexColFace[j*9+1] = vertexCol[v1i*3+1];
								vertexColFace[j*9+2] = vertexCol[v1i*3+2];

								vertexColFace[j*9+3] = vertexCol[v2i*3+0];
								vertexColFace[j*9+4] = vertexCol[v2i*3+1];
								vertexColFace[j*9+5] = vertexCol[v2i*3+2];

								vertexColFace[j*9+6] = vertexCol[v3i*3+0];
								vertexColFace[j*9+7] = vertexCol[v3i*3+1];
								vertexColFace[j*9+8] = vertexCol[v3i*3+2];
							}

							if (vertexTex != null) {
								vertexTexFace[j*6+0] = vertexTex[v1i*2+0];
								vertexTexFace[j*6+1] = vertexTex[v1i*2+1];

								vertexTexFace[j*6+2] = vertexTex[v2i*2+0];
								vertexTexFace[j*6+3] = vertexTex[v2i*2+1];

								vertexTexFace[j*6+4] = vertexTex[v3i*2+0];
								vertexTexFace[j*6+5] = vertexTex[v3i*2+1];
							}

						}
					}
				}

			}

			} catch (IOException e) {
				Log.w("avdl", "ReadPly: Error while parsing '" +assetName);
				return null;
			}

		}

		return new Object[] {
			vertexPosFace != null ? vertexPosFace : null,
			vertexColFace != null ? vertexColFace : null,
			vertexTexFace != null ? vertexTexFace : null,
		};
	}

	static int PlayAudio(String resourceName, int loop) {

		int id = AvdlActivity.context.getApplicationContext().getResources().getIdentifier(
			resourceName, "raw", AvdlActivity.context.getPackageName()
		);
		if (id == 0) {
			Log.w("avdl", "no resource id for " +resourceName);
			return -1;
		}

		// find index for this audio
		int desiredIndex = -1;
		for (int i = 0; i < 5; i++) {
			if (mediaPlayer[i] == null) {
				desiredIndex = i;
				break;
			}
			// previous sound already there, clear it
			else
			if (!mediaPlayer[i].isPlaying()) {
				StopAudio(i);
				desiredIndex = i;
				break;
			}
		}

		// no index available - too many sounds playing
		if (desiredIndex == -1) {
			Log.w("avdl", "too many sounds playing");
			return -1;
		}

		MediaPlayer mp = MediaPlayer.create(AvdlActivity.context, id);
		if (loop == 1) {
			mp.setLooping(true);
		}
		mediaPlayer[desiredIndex] = mp;
		mp.start();

		// on complete, remove media player
		mp.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
			public void onCompletion(MediaPlayer mp) {
				for (int i = 0; i < 5; i++) {
					mp.release();
					if (mediaPlayer[i] == mp) {
						mediaPlayer[i] = null;
					}
				}
			}
		});

		return desiredIndex;

	}

	static void StopAudio(int index) {
		if (index < 0 || index >= 5) {
			return;
		}
		mediaPlayer[index].stop();
		mediaPlayer[index].release();
		mediaPlayer[index] = null;
	}

}

class PlyElement {
	String name;
	int amount;
	List<PlyProperty> properties;

	PlyElement(String givenName, int givenAmount) {
		name = givenName;
		amount = givenAmount;
		properties = new ArrayList<PlyProperty>();
	}
}

class PlyProperty {
	String name;
	String type;
	boolean isList;
	String quantity;

	PlyProperty(String gName, String gType, boolean gIsList, String gQuantity) {
		name = gName;
		type = gType;
		isList = gIsList;
		quantity = gQuantity;
	}
}
