#include <stdio.h>
#include <unistd.h>
#include "sound_lib.h"

int main(int argc, char *argv[])
{
/*	
	memset(child_proc, 0x00, sizeof(play_pid_info) * MAX_PROC);
	int cmd = 0;
	pid_t tmp_pid;
	
	play_wave_info wave_info;
	
	int status = 0;
	int cnt = 0;
	while(1)
	{
		scanf("%d", &cmd);
		switch (cmd)
		{
			case 1:
				if (count < MAX_PROC)
				{
					child_proc[count].pid = getpid();
					child_proc[count].status = STATUS_PLAY_ONE_MORE;
					child_proc[count].repeat_cnt = LOOP_CAST;
					strncpy(child_proc[count++].filename, argv[1], strlen(argv[1]));
					printf("wav?t?@?C???ﾄ??\n");
					
					tmp_pid = fork();
					if (tmp_pid < 0)
					{
						printf("?q?v???Z?X?쐬???s???܂????B\n");
						break;
					}
					else if (0 == tmp_pid)
					{
					
						
						printf("filename:%s\n", child_proc[0].filename);
						status = STATUS_PLAY_ONE_MORE;
						cnt = LOOP_CAST;
						printf("pid:%d\n", child_proc[0].pid);
						if (wave_read_file_header(child_proc[0].filename, &wave_info) == -1)
						{
							printf("WAV?t?@?C???ǂݍ??ނɂ͎??s???܂????B\n", getpid());
						}
						printf("count: %d\n", count);
						dsp_play(&wave_info);
					}
					else
					{
						break;
					}
				}
				else
				{
					printf("?v???Z?X???͍ő吧?ɂȂ閧ﾜ?????B\n");
				}
				break;
			case 2:
				printf("pid:%d\n", child_proc[0].pid);
				
				if (0 == count)
				{
					printf("?Đ??????v???Z?X???閧ﾜ???B\n");
					break;
				}
				if ( (child_proc[0].status & STATUS_PLAY_SUSPEND) == 1 )
				{
					printf("?Đ??????v???Z?X?͒??~???黷ﾜ?????B\n");
					break;
				}
				
				if ( (child_proc[0].status & STATUS_PLAY_MASK) == 1 )
				{
					//child_proc[0].status = 0;
					child_proc[0].status |= STATUS_PLAY_SUSPEND;
				}
				break;
			case 3:
				if (0 == count)
				{
					printf("?Đ??????v???Z?X???閧ﾜ???B\n");
					break;
				}
				if ( (child_proc[0].status & STATUS_PLAY_MASK) == 1 )
				{
					printf("?v???Z?X?͍??Đ????ł??B\n");
					break;
				}
				
				if ( (child_proc[0].status & STATUS_PLAY_SUSPEND) == 1 )
				{
					//child_proc[0].status = 0;
					child_proc[0].status &= ~STATUS_PLAY_SUSPEND;
					child_proc[0].status |= STATUS_PLAY_REBROADCAST;
				}
			default:
				printf("default.\n");
				break;
		}
				
	}
	*/
	
	int refresh_times = 0;
	
	printf("再生回数を入力してください:\n");
	
	scanf("%d", &refresh_times);
	
	printf("再生回数は%d回\n", refresh_times);
	
	if ( -1 == play_wave(argv[1], refresh_times))
	{
		return -1;
	}
	
	
	return 0;
}
