#include <stdio.h>
#include <stdint.h>
#include <alsa/asoundlib.h>

#define BUF_SIZE 2048

static int16_t buf[BUF_SIZE];

static void fill_buf()
{
    for (int i = 0; i < BUF_SIZE; i++)
    {
        if (i & 128)
            buf[i] = INT16_MAX;
        else
            buf[i] = INT16_MIN;
    }
}

void main(void)
{
    snd_pcm_t *handle = NULL;
    snd_output_t *output = NULL;
    snd_pcm_hw_params_t *hw_params = NULL;
    snd_pcm_sw_params_t *sw_params = NULL;
    snd_pcm_status_t *status = NULL;
    int err;

    fill_buf();

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_sw_params_alloca(&sw_params);
    snd_pcm_status_alloca(&status);

    err = snd_output_stdio_attach(&output, stdout, 0);
    err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    //
    // HW params
    //
    err = snd_pcm_hw_params_any(handle, hw_params);
    printf("\n==== HW parameters any ===== \n\n");
    err = snd_pcm_hw_params_dump(hw_params, output);

    err = snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    err = snd_pcm_hw_params_set_rate_resample(handle, hw_params, 1);
    err = snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16);
    err = snd_pcm_hw_params_set_channels(handle, hw_params, 1);

    int dir;
    unsigned int rate = 44100;
    err = snd_pcm_hw_params_set_rate_near(handle, hw_params, &rate, &dir);
    unsigned int buffer_time = 500000;    /* ring buffer length in us */
    err = snd_pcm_hw_params_set_buffer_time_near(handle, hw_params, &buffer_time, &dir);
    snd_pcm_uframes_t buffer_size;
    err = snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);

    unsigned int period_time = 100000;
    // Buffer should contain at least two periods
    err = snd_pcm_hw_params_set_period_time_near(handle, hw_params, &period_time, &dir);
    snd_pcm_uframes_t period_size;
    err = snd_pcm_hw_params_get_period_size(hw_params, &period_size, &dir);

    printf("\n==== HW parameters set up ===== \n\n");
    err = snd_pcm_hw_params_dump(hw_params, output);
    err = snd_pcm_hw_params(handle, hw_params); // write hw params to device

    //
    // SW params
    //
    err = snd_pcm_sw_params_current(handle, sw_params);
    printf("\n==== SW parameters get current ===== \n\n");
    snd_pcm_sw_params_dump(sw_params, output);
    err = snd_pcm_sw_params_set_start_threshold(handle, sw_params, (buffer_size / period_size) * period_size);
    err = snd_pcm_sw_params_set_avail_min(handle, sw_params, period_size);

    printf("\n==== SW parameters set up ===== \n\n");
    snd_pcm_sw_params_dump(sw_params, output);
    err = snd_pcm_sw_params(handle, sw_params); // write sw params to device

    //
    // Status
    //
    printf("\n==== Status dump ===== \n\n");
    err = snd_pcm_status(handle, status);
    snd_pcm_status_dump(status, output);

    //
    // Write
    //
    snd_pcm_sframes_t frames_num;
    frames_num = snd_pcm_avail(handle);
    int counter = 20;
    while (counter >= 0)
    {
        frames_num = snd_pcm_writei(handle, buf, BUF_SIZE);
        frames_num = snd_pcm_avail(handle);
        printf("--- ----\n");
        err = snd_pcm_status(handle, status);
        snd_pcm_status_dump(status, output);
        counter--;
    }

    err = snd_pcm_drain(handle);
    printf("--- Drained ---- %s\n", snd_strerror(err));
    err = snd_pcm_status(handle, status);
    snd_pcm_status_dump(status, output);

    err = snd_pcm_prepare(handle);
    printf("--- Prepared again ----\n");
    err = snd_pcm_status(handle, status);
    snd_pcm_status_dump(status, output);

    printf("\n==== PCM dump ===== \n\n");
    snd_pcm_dump(handle, output);

    snd_pcm_close(handle);
}
