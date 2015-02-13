#include <alsa/asoundlib.h>
static char *device = "default";                        /* playback device */
snd_output_t *output = NULL;
unsigned char buffer[16*1024];                          /* some random data */

int main(void)
{
        int err;
        unsigned int i;
        snd_pcm_t *handle;
        snd_pcm_sframes_t frames;
        //FILE*fp;
        //fp = fopen("test.wav", "r");
        for (i = 0; i < sizeof(buffer); i++)
                buffer[i] = random() & 0xff;
        if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
                printf("Playback open error: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
        }
        
        if ((err = snd_pcm_set_params(handle,
                                      SND_PCM_FORMAT_U16_LE,
                                      //SND_PCM_FORMAT_U8,
                                      SND_PCM_ACCESS_RW_INTERLEAVED,
                                      2,
                                      44100,
                                      1,
                                      500000)) < 0) {   /* 0.5sec */
                printf("Playback open error: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
        }
        #if 0
        if ((err = snd_pcm_set_params(handle,
                                      SND_PCM_FORMAT_U8,
                                      SND_PCM_ACCESS_RW_INTERLEAVED,
                                      1,
                                      48000,
                                      1,
                                      500000)) < 0) {   /* 0.5sec */
                printf("Playback open error: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
        }
        #endif
       // int m = 0;
       // while(!feof(fp))
       // {
        //	m = fread(buffer, 1, sizeof(buffer), fp);
        for (i = 0; i < 16; i++) {
                frames = snd_pcm_writei(handle, buffer, sizeof(buffer) / 4);
                if (frames < 0)
                        frames = snd_pcm_recover(handle, frames, 0);
                if (frames < 0) {
                        printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
                        break;
                }
                if (frames > 0 && frames < (long)sizeof(buffer))
                        printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
        }
        snd_pcm_close(handle);
        //fclose(fp);
        return 0;
}
