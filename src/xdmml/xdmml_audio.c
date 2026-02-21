/*
 * xdmml_audio.c - XDMML Audio Implementation
 */

#include "xdmml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    #include <alsa/asoundlib.h>
    #define XDMML_HAS_ALSA 1
#elif defined(XDMML_PLATFORM_MACOS)
    #include <AudioToolbox/AudioToolbox.h>
    #define XDMML_HAS_COREAUDIO 1
#endif

// ─── Audio Structure ─────────────────────────────────────────────────────────

struct XDMML_Audio {
#ifdef XDMML_HAS_ALSA
    snd_pcm_t* pcm_handle;
#elif defined(XDMML_HAS_COREAUDIO)
    AudioQueueRef queue;
#endif
    XDMML_AudioSpec spec;
    bool is_playing;
};

// ─── Audio Functions ─────────────────────────────────────────────────────────

XDMML_Audio* xdmml_open_audio(XDMML_AudioSpec* desired, XDMML_AudioSpec* obtained) {
    if (!desired) return NULL;
    
    XDMML_Audio* audio = (XDMML_Audio*)calloc(1, sizeof(XDMML_Audio));
    if (!audio) return NULL;
    
    audio->spec = *desired;
    
#ifdef XDMML_HAS_ALSA
    int err = snd_pcm_open(&audio->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "[XDMML] Cannot open audio device: %s\n", snd_strerror(err));
        free(audio);
        return NULL;
    }
    
    // Set parameters
    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(audio->pcm_handle, params);
    snd_pcm_hw_params_set_access(audio->pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(audio->pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(audio->pcm_handle, params, desired->channels);
    
    unsigned int rate = desired->frequency;
    snd_pcm_hw_params_set_rate_near(audio->pcm_handle, params, &rate, 0);
    snd_pcm_hw_params_set_buffer_size(audio->pcm_handle, params, desired->samples * 4);
    
    err = snd_pcm_hw_params(audio->pcm_handle, params);
    if (err < 0) {
        fprintf(stderr, "[XDMML] Cannot set audio parameters: %s\n", snd_strerror(err));
        snd_pcm_close(audio->pcm_handle);
        free(audio);
        return NULL;
    }
    
    if (obtained) {
        *obtained = audio->spec;
        obtained->frequency = rate;
    }
#endif
    
    return audio;
}

void xdmml_close_audio(XDMML_Audio* audio) {
    if (!audio) return;
    
#ifdef XDMML_HAS_ALSA
    if (audio->pcm_handle) {
        snd_pcm_drain(audio->pcm_handle);
        snd_pcm_close(audio->pcm_handle);
    }
#endif
    
    free(audio);
}

void xdmml_play_audio(XDMML_Audio* audio) {
    if (!audio) return;
    audio->is_playing = true;
    
#ifdef XDMML_HAS_ALSA
    snd_pcm_prepare(audio->pcm_handle);
#endif
}

void xdmml_pause_audio(XDMML_Audio* audio) {
    if (!audio) return;
    audio->is_playing = false;
    
#ifdef XDMML_HAS_ALSA
    snd_pcm_pause(audio->pcm_handle, 1);
#endif
}

// ─── WAV Loading ─────────────────────────────────────────────────────────────

typedef struct {
    char riff[4];
    uint32_t file_size;
    char wave[4];
} WAV_Header;

typedef struct {
    char fmt[4];
    uint32_t chunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} WAV_Format;

typedef struct {
    char data[4];
    uint32_t data_size;
} WAV_Data;

XDMML_Result xdmml_load_wav(const char* filename, XDMML_AudioSpec* spec,
                             uint8_t** buffer, uint32_t* length) {
    if (!filename || !spec || !buffer || !length) {
        return XDMML_ERROR_INVALID_PARAM;
    }
    
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "[XDMML] Cannot open WAV file: %s\n", filename);
        return XDMML_ERROR;
    }
    
    // Read WAV header
    WAV_Header header;
    WAV_Format format;
    WAV_Data data_header;
    
    fread(&header, sizeof(WAV_Header), 1, fp);
    fread(&format, sizeof(WAV_Format), 1, fp);
    
    // Find data chunk
    while (1) {
        fread(&data_header, 8, 1, fp);
        if (strncmp(data_header.data, "data", 4) == 0) {
            break;
        }
        fseek(fp, data_header.data_size, SEEK_CUR);
    }
    
    // Fill spec
    spec->frequency = format.sample_rate;
    spec->channels = format.num_channels;
    spec->samples = 4096;
    
    if (format.bits_per_sample == 16) {
        spec->format = XDMML_AUDIO_S16;
    } else {
        spec->format = XDMML_AUDIO_F32;
    }
    
    // Allocate buffer and read data
    *length = data_header.data_size;
    *buffer = (uint8_t*)malloc(*length);
    if (!*buffer) {
        fclose(fp);
        return XDMML_ERROR_OUT_OF_MEMORY;
    }
    
    fread(*buffer, 1, *length, fp);
    fclose(fp);
    
    return XDMML_OK;
}

void xdmml_free_wav(uint8_t* buffer) {
    free(buffer);
}
