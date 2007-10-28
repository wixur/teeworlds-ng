#include <engine/system.h>
#include <engine/interface.h>
#include <engine/config.h>

#include <engine/external/portaudio/portaudio.h>
#include <engine/external/wavpack/wavpack.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

enum
{
	NUM_SAMPLES = 512,
	NUM_VOICES = 64,
	NUM_CHANNELS = 16,
	
	MAX_FRAMES = 1024
};

typedef struct
{
	short *data;
	int num_frames;
	int rate;
	int channels;
	int loop_start;
	int loop_end;
} SAMPLE;

typedef struct
{
	int vol;
	int pan;
} CHANNEL;

typedef struct VOICE_t
{
	SAMPLE *snd;
	CHANNEL *channel;
	int tick;
	int vol; /* 0 - 255 */
	int flags;
	int x, y;
} VOICE;

static SAMPLE samples[NUM_SAMPLES] = { {0} };
static VOICE voices[NUM_VOICES] = { {0} };
static CHANNEL channels[NUM_CHANNELS] = { {255, 0} };

static LOCK sound_lock = 0;

static int center_x = 0;
static int center_y = 0;

static int mixing_rate = 48000;

void snd_set_channel(int cid, float vol, float pan)
{
	channels[cid].vol = (int)(vol*255.0f);
	channels[cid].pan = (int)(pan*255.0f); /* TODO: this is only on and off right now */
}

static int play(int cid, int sid, int flags, float x, float y)
{
	int vid = -1;
	int i;
	
	lock_wait(sound_lock);
	
	/* search for voice */
	/* TODO: fix this linear search */
	for(i = 0; i < NUM_VOICES; i++)
	{
		if(!voices[i].snd)
		{
			vid = i;
			break;
		}
	}
	
	/* voice found, use it */
	if(vid != -1)
	{
		voices[vid].snd = &samples[sid];
		voices[vid].channel = &channels[cid];
		voices[vid].tick = 0;
		voices[vid].vol = 255;
		voices[vid].flags = flags;
		voices[vid].x = (int)x;
		voices[vid].y = (int)y;
	}
	
	lock_release(sound_lock);
	return vid;
}

int snd_play_at(int cid, int sid, int flags, float x, float y)
{
	return play(cid, sid, flags|SNDFLAG_POS, x, y);
}

int snd_play(int cid, int sid, int flags)
{
	return play(cid, sid, flags, 0, 0);
}

void snd_stop(int vid)
{
	/* TODO: a nice fade out */
	lock_wait(sound_lock);
	voices[vid].snd = 0;
	lock_release(sound_lock);
}

/* TODO: there should be a faster way todo this */
static short int2short(int i)
{
	if(i > 0x7fff)
		return 0x7fff;
	else if(i < -0x7fff)
		return -0x7fff;
	return i;
}

static int iabs(int i)
{
	if(i<0)
		return -i;
	return i;
}

static void mix(short *final_out, unsigned frames)
{
	int mix_buffer[MAX_FRAMES*2] = {0};
	int i, s;

	/* aquire lock while we are mixing */
	lock_wait(sound_lock);
	
	for(i = 0; i < NUM_VOICES; i++)
	{
		if(voices[i].snd)
		{
			/* mix voice */
			VOICE *v = &voices[i];
			int *out = mix_buffer;

			int step = v->snd->channels; /* setup input sources */
			short *in_l = &v->snd->data[v->tick*step];
			short *in_r = &v->snd->data[v->tick*step+1];
			
			int end = v->snd->num_frames-v->tick;

			int rvol = v->channel->vol;
			int lvol = v->channel->vol;

			/* make sure that we don't go outside the sound data */
			if(frames < end)
				end = frames;
			
			/* check if we have a mono sound */
			if(v->snd->channels == 1)
				in_r = in_l;

			/* volume calculation */
			if(v->flags&SNDFLAG_POS && v->channel->pan)
			{
				/* TODO: we should respect the channel panning value */
				const int range = 1500; /* magic value, remove */
				int dx = v->x - center_x;
				int dy = v->y - center_y;
				int dist = sqrt(dx*dx+dy*dy); /* double here. nasty */
				int p = iabs(dx);
				if(dist < range)
				{
					/* panning */
					if(dx > 0)
						lvol = ((range-p)*lvol)/range;
					else
						rvol = ((range-p)*rvol)/range;
					
					/* falloff */
					lvol = (lvol*(range-dist))/range;
					rvol = (rvol*(range-dist))/range;
				}
			}

			/* process all frames */
			for(s = 0; s < end; s++)
			{
				*out++ += (*in_l)*lvol;
				*out++ += (*in_r)*rvol;
				in_l += step;
				in_r += step;
				v->tick++;
			}
			
			/* free voice if not used any more */
			if(v->tick == v->snd->num_frames)
				v->snd = 0;
			
		}
	}

	/* release the lock */
	lock_release(sound_lock);

	/* clamp accumulated values */
	/* TODO: this seams slow */
	for(i = 0; i < frames; i++)
	{
		int j = i<<1;
		int vl = mix_buffer[j]>>8;
		int vr = mix_buffer[j+1]>>8;

		final_out[j] = int2short(vl);
		final_out[j+1] = int2short(vr);
	}	
}

static int pacallback(const void *in, void *out, unsigned long frames, const PaStreamCallbackTimeInfo* time, PaStreamCallbackFlags status, void *user)
{
	mix(out, frames);
	return 0;
}

static PaStream *stream;

int snd_init()
{
	PaStreamParameters params;
	PaError err = Pa_Initialize();
	
	mixing_rate = config.snd_rate;

	sound_lock = lock_create();

	params.device = Pa_GetDefaultOutputDevice();
	if(params.device < 0)
		return 1;
	params.channelCount = 2;
	params.sampleFormat = paInt16;
	params.suggestedLatency = Pa_GetDeviceInfo(params.device)->defaultLowOutputLatency;
	params.hostApiSpecificStreamInfo = 0x0;


	err = Pa_OpenStream(
			&stream,        /* passes back stream pointer */
			0,              /* no input channels */
			&params,                /* pointer to parameters */
			mixing_rate,          /* sample rate */
			128,            /* frames per buffer */
			paClipOff,              /* no clamping */
			pacallback,             /* specify our custom callback */
			0x0); /* pass our data through to callback */
	err = Pa_StartStream(stream);

	return 0;
}

int snd_shutdown()
{
	Pa_StopStream(stream);
	Pa_Terminate();

	lock_destroy(sound_lock);

	return 0;
}

int snd_alloc_id()
{
	/* TODO: linear search, get rid of it */
	unsigned sid;
	for(sid = 0; sid < NUM_SAMPLES; sid++)
	{
		if(samples[sid].data == 0x0)
			return sid;
	}

	return -1;
}

static void rate_convert(int sid)
{
	SAMPLE *snd = &samples[sid];
	int num_frames = 0;
	short *new_data = 0;
	int i;
	
	/* make sure that we need to convert this sound */
	if(!snd->data || snd->rate == mixing_rate)
		return;

	/* allocate new data */
	num_frames = (int)((snd->num_frames/(float)snd->rate)*mixing_rate);
	new_data = mem_alloc(num_frames*snd->channels*sizeof(short), 1);
	
	for(i = 0; i < num_frames; i++)
	{
		/* resample TODO: this should be done better, like linear atleast */
		float a = i/(float)num_frames;
		int f = (int)(a*snd->num_frames);
		if(f >= snd->num_frames)
			f = snd->num_frames-1;
		
		/* set new data */
		if(snd->channels == 1)
			new_data[i] = snd->data[f];
		else if(snd->channels == 2)
		{
			new_data[i*2] = snd->data[f*2];
			new_data[i*2+1] = snd->data[f*2+1];
		}
	}
	
	/* free old data and apply new */
	mem_free(snd->data);
	snd->data = new_data;
	snd->num_frames = num_frames;
}


static FILE *file = NULL;

static int read_data(void *buffer, int size)
{
	return fread(buffer, 1, size, file);	
}

int snd_load_wv(const char *filename)
{
	SAMPLE *snd;
	int sid = -1;
	char error[100];
	WavpackContext *context;
	
	/* don't waste memory on sound when we are stress testing */
	if(config.stress)
		return -1;

	file = fopen(filename, "rb"); /* TODO: use system.h stuff for this */
	if(!file)
	{
		dbg_msg("sound/wv", "failed to open %s", filename);
		return -1;
	}

	sid = snd_alloc_id();
	if(sid < 0)
		return -1;
	snd = &samples[sid];

	context = WavpackOpenFileInput(read_data, error);
	if (context)
	{
		int samples = WavpackGetNumSamples(context);
		int bitspersample = WavpackGetBitsPerSample(context);
		unsigned int samplerate = WavpackGetSampleRate(context);
		int channels = WavpackGetNumChannels(context);
		int *data;
		int *src;
		short *dst;
		int i;

		snd->channels = channels;
		snd->rate = samplerate;

		if(snd->channels > 2)
		{
			dbg_msg("sound/wv", "file is not mono or stereo. filename='%s'", filename);
			return -1;
		}

		if(snd->rate != 44100)
		{
			dbg_msg("sound/wv", "file is %d Hz, not 44100 Hz. filename='%s'", snd->rate, filename);
			return -1;
		}
		
		if(bitspersample != 16)
		{
			dbg_msg("sound/wv", "bps is %d, not 16, filname='%s'", bitspersample, filename);
			return -1;
		}

		data = (int *)mem_alloc(4*samples*channels, 1);
		WavpackUnpackSamples(context, data, samples); /* TODO: check return value */
		src = data;
		
		snd->data = (short *)mem_alloc(2*samples*channels, 1);
		dst = snd->data;

		for (i = 0; i < samples*channels; i++)
			*dst++ = (short)*src++;

		mem_free(data);

		snd->num_frames = samples;
		snd->loop_start = -1;
		snd->loop_end = -1;
	}
	else
	{
		dbg_msg("sound/wv", "failed to open %s: %s", filename, error);
	}

	fclose(file);
	file = NULL;

	if(config.debug)
		dbg_msg("sound/wv", "loaded %s", filename);

	rate_convert(sid);
	return sid;
}

void snd_set_master_volume(float vol)
{
	/*master_vol = vol;*/
}

void snd_set_listener_pos(float x, float y)
{
	center_x = (int)x;
	center_y = (int)y;
}
