#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>


static void disp_element(snd_mixer_t *handler,
						 snd_mixer_elem_t *elem,
						 snd_mixer_elem_t *last_elem,
						 snd_mixer_selem_id_t *sid);
						
						
static void set_volumn(snd_mixer_t *handler,
					   snd_mixer_elem_t *elem,
					   char *mixer_name,
				       snd_mixer_selem_id_t *sid);


int main(int argc, char *argv[])
{
	snd_mixer_t *handler;
	snd_mixer_elem_t *elem;
	snd_mixer_elem_t *last_elem;
	snd_mixer_selem_id_t *sid;
	
	snd_mixer_open(&handler, 0);
  	snd_mixer_attach(handler, "default");
  	snd_mixer_selem_register(handler, NULL, NULL);
 	snd_mixer_load(handler);
#if 0
  	/* 取得第一個 element，也就是 Master */
  	elem = snd_mixer_first_elem(handler);
	last_elem = snd_mixer_last_elem(handler);
	snd_mixer_selem_id_alloca(&sid);
	
	char *e_name, *i_name ;
	unsigned int e_index, i_index;
	int i = 0;
	for(i = 0;elem != last_elem; i++)
	{
		snd_mixer_selem_get_id(elem, sid);
		
		
		i_index = snd_mixer_selem_id_get_index(sid);
		i_name = snd_mixer_selem_id_get_name(sid);
		e_name = snd_mixer_selem_get_name(elem);
		e_index = snd_mixer_selem_get_index(elem);
		
		printf("No %d:\n", i);
		printf("i_index:%d\n", i_index);
		printf("i_name:%s\n", i_name);
		printf("e_index:%d\n", e_index);
		printf("e_name:%s\n", e_name);
		
		printf("\n\n");
		
		elem = snd_mixer_elem_next(elem);
	}
#endif


	disp_element(handler, elem, last_elem, sid);	
	
	set_volumn(handler, elem, "Master", sid);
	
	set_volumn(handler, elem, "PCM", sid);
	
	return 0;
}


static void disp_element(snd_mixer_t *handler,
						snd_mixer_elem_t *elem,
						snd_mixer_elem_t *last_elem,
						snd_mixer_selem_id_t *sid)
{
	/* 取得第一個 element，也就是 Master */
  	elem = snd_mixer_first_elem(handler);
	last_elem = snd_mixer_last_elem(handler);
	snd_mixer_selem_id_alloca(&sid);
	
	char *e_name, *i_name ;
	unsigned int e_index, i_index;
	int i = 0;
	for(i = 0;elem != last_elem; i++)
	{
		snd_mixer_selem_get_id(elem, sid);
		
		
		i_index = snd_mixer_selem_id_get_index(sid);
		i_name = (char *)snd_mixer_selem_id_get_name(sid);
		e_name = (char *)snd_mixer_selem_get_name(elem);
		e_index = snd_mixer_selem_get_index(elem);
		
		printf("No %d:\n", i);
		printf("i_index:%d\n", i_index);
		printf("i_name:%s\n", i_name);
		printf("e_index:%d\n", e_index);
		printf("e_name:%s\n", e_name);
		
		printf("\n\n");
		
		elem = snd_mixer_elem_next(elem);
	}
}


static void set_volumn(snd_mixer_t *handler,
					   snd_mixer_elem_t *elem,
					   char *mixer_name,
				       snd_mixer_selem_id_t *sid)
{
	long min_vol, max_vol;
	int mix_index = 0;
	char *mix_name = mixer_name;
	long set_vol_left = 40;
	long set_vol_right = 0;
	
	snd_mixer_selem_id_alloca(&sid);
	
	snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);
    
	/*Masterエリメントを設定する*/
	elem = snd_mixer_find_selem(handler, sid);
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printf("[Before]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
	
	set_vol_left = (float)set_vol_left / 100 * (max_vol - min_vol) + min_vol;
	set_vol_right = (float)set_vol_right / 100 * (max_vol - min_vol) + min_vol;
/*	
	snd_mixer_selem_set_playback_volume_range(elem, 0, 100);
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printf("[After]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
*/	
	if (snd_mixer_selem_has_playback_switch(elem))
	{
		if (!snd_mixer_selem_has_playback_switch_joined(elem))
		{
			snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, 1);
		}
		snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, 1);
	}
	
	
	snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, set_vol_right);
	snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, set_vol_left);
	
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printf("[Before END]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
}
