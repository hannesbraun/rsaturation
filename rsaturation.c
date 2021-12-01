// Copyright Hannes Braun 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt)

#include <lv2/core/lv2.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define SAT_URI "https://hannesbraun.net/ns/lv2/rsaturation"

typedef enum {
	INPUT_L = 0,
	INPUT_R = 1,
	OUTPUT_L = 2,
	OUTPUT_R = 3,
	SATURATION = 4
} PortIndex;

typedef struct {
	// Port buffers
	const float* sat;
	const float* input_l;
	const float* input_r;
	float* output_l;
	float* output_r;
} Saturation;

static LV2_Handle instantiate(
	const LV2_Descriptor* descriptor,
	double rate,
	const char* bundle_path,
	const LV2_Feature* const* features)
{
	Saturation* s = (Saturation*) calloc(1, sizeof(Saturation));
	return (LV2_Handle) s;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
	Saturation* s = (Saturation*) instance;

	switch ((PortIndex) port) {
	case INPUT_L:
		s->input_l = (const float*) data;
		break;
	case INPUT_R:
		s->input_r = (const float*) data;
		break;
	case OUTPUT_L:
		s->output_l = (float*) data;
		break;
	case OUTPUT_R:
		s->output_r = (float*) data;
		break;
	case SATURATION:
		s->sat = (const float*) data;
		break;
	}
}

static void activate(LV2_Handle instance) {}

static void run(LV2_Handle instance, uint32_t n_samples) {
	Saturation* s = (Saturation*) instance;

	const float* const input_l = s->input_l;
	const float* const input_r = s->input_r;
	float* const output_l = s->output_l;
	float* const output_r = s->output_r;
	const float sat = (*(s->sat) * M_PI) / 200.0;
	const float sin_sat = sin(sat);

	const int do_sat = sat != 0.0 && sin_sat != 0.0;

	float l, r;
	for (uint32_t pos = 0; pos < n_samples; pos++) {
		// Clip signal
		l = fmax(fmin(input_l[pos], 1.0), -1.0);
		r = fmax(fmin(input_r[pos], 1.0), -1.0);

		// Saturate
		if (do_sat) {
			l = sin(l * sat) / sin_sat;
			r = sin(r * sat) / sin_sat;
		}

		output_l[pos] = l;
		output_r[pos] = r;
	}
}

static void deactivate(LV2_Handle instance) {}

static void cleanup(LV2_Handle instance) {
	free(instance);
}

static const void* extension_data(const char* uri) {
	return NULL;
}

static const LV2_Descriptor descriptor = {
	SAT_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
	return index == 0 ? &descriptor : NULL;
}
