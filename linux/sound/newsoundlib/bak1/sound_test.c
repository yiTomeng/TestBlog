#include <stdio.h>
#include "sound_lib.h"



int main(int argc, char *argv[])
{
	int left_vol, right_vol;
	
	//printf("ボリュウムを入力してください:\n");
	
	//scanf("%d %d", &left_vol, &right_vol);
	
	left_vol = 80;
	right_vol = 80;
	/*
	set_volumn("Master", left_vol, right_vol);

	set_volumn("PCM", left_vol, right_vol);
	*/
	
	snd_pcm_t *handler = NULL;
	
	open_device(&handler);
	
	assert(handler != NULL);
	
	play_file(handler, "../test3.wav", left_vol, right_vol);

	close_device(handler);
	
	return 0;	
}
