#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "../../simpleimg/include/simpleimg.h"
#include "../include/simple.h"

static inline void blend_alpha(uint8_t *v, uint8_t w) {
	if (w > *v) {*v = w;}
}

static inline void blend_alpha_erase(uint8_t *v, uint8_t w) {
	if (w < *v) {*v = w;}
}

static float f01cap(float x) {
	if (x < 0.0f) { return 0.0; }
	if (x > 1.0f) { return 1.0; }
	return x;
}

static uint8_t f2u(float x) {
	if (x < 0.0f) { return 0; }
	if (x >= 1.0f) { return 255; }
	// this never overflow
	// as long as int has smaller bits than the fixed part of float
	return (uint8_t)(x * 256.0f);
}

static uint32_t cap(float x, uint32_t y) {
	if (x < 0) { return 0; }
	if ((uint32_t)x >= y) {
		return (y - 1);
	}
	return (uint32_t)x;
}

static void ecircle(Simpleimg* img, float x, float y,
	uint32_t i, uint32_t j, float size, float alpha)
{
	float dx = (float)i + 0.5f - x;
	float dy = (float)j + 0.5f - y;
	float dr = sqrtf(dx * dx + dy * dy) - size;
	if (dr >= 0.5f) { return; }
	uint8_t* p = simpleimg_offset(img, i, j);
	if (fabsf(dr) < 0.5f) {
		blend_alpha_erase(p + 3,
			f2u(alpha * (0.5f - dr)));
		return;
	}
	blend_alpha_erase(p + 3, f2u(alpha));
}

static void bcircle(Simpleimg* img, float x, float y,
	uint32_t i, uint32_t j, float size, float alpha, uint8_t color[3])
{
	float dx = (float)i + 0.5f - x;
	float dy = (float)j + 0.5f - y;
	float dr = sqrtf(dx * dx + dy * dy) - size;
	if (dr >= 0.5f) { return; }
	uint8_t* p = simpleimg_offset(img, i, j);
	memcpy(p, color, sizeof(uint8_t) * 3);
	if (fabsf(dr) < 0.5f) {
		blend_alpha(p + 3, f2u(alpha * (0.5f - dr)));
		return;
	}
	blend_alpha(p + 3, f2u(alpha));
}

static void fcircle(Simpleimg* img, float x, float y,
	float size, float alpha, uint8_t color[3], bool eraser) {
	float xmaxf = ceilf(x + size);
	float xminf = floorf(x - size);
	float ymaxf = ceilf(y + size);
	float yminf = floorf(y - size);
	uint32_t xmax = cap(xmaxf, img->width);
	uint32_t xmin = cap(xminf, img->width);
	uint32_t ymax = cap(ymaxf, img->height);
	uint32_t ymin = cap(yminf, img->height);
	for (uint32_t i = xmin; i < xmax; i += 1) {
		for (uint32_t j = ymin; j < ymax; j += 1) {
			if (eraser) {
				ecircle(img, x, y, i, j, size, alpha);
			} else {
				bcircle(img, x, y, i, j, size, alpha, color);
			}
		}
	}
}

static void sib_simple_update2(SibSimple *sib, Simpleimg *img,
	float x2, float y2, float p)
{
	float alpha1 = sib->alpha_k * sib->pp + sib->alpha_b;
	float alpha2 = sib->alpha_k * sib->pp + sib->alpha_b;
	float size1 = sib->size_k * sib->pp + sib->size_b;
	float size2 = sib->size_k * sib->pp + sib->size_b;
	float dx = x2 - sib->px;
	float dy = y2 - sib->py;
	float ds = size2 - size1;
	float da = alpha2 - alpha1;
	float dist = sqrtf(dx * dx + dy * dy);
	float t = 0.0;
	while (t < 1.0f) {
		float x = dx * t + sib->px;
		float y = dy * t + sib->py;
		float s = ds * t + size1;
		float a = da * t + alpha1;
		fcircle(img, x, y, s, f01cap(a), sib->color, sib->eraser);
		float spacing = sib->spacing * s;
		if (spacing < sib->spacing) { spacing = sib->spacing; }
		t += spacing / dist;
	}
}

// default config
void sib_simple_config(SibSimple *sib) {
	sib->spacing = 0.25f;
	sib->alpha_k = 0.1f;
	sib->alpha_b = 0.6f;
	sib->size_k = 10.0f;
	sib->size_b = 0.0;
	sib->color[0] = 255;
	sib->color[1] = 0;
	sib->color[2] = 0;
	sib->eraser = false;
}

void sib_simple_config_eraser(SibSimple *sib) {
	sib->spacing = 0.25f;
	sib->alpha_k = 0.0f;
	sib->alpha_b = 0.0f;
	sib->size_k = 10.0f;
	sib->size_b = 0.0;
	sib->eraser = true;
}

void sib_simple_update(SibSimple *sib, Simpleimg *img,
	float x, float y, float p)
{
	if (sib->drag) {
		sib_simple_update2(sib, img, x, y, p);
	}
	sib->px = x;
	sib->py = y;
	sib->pp = p;
	sib->drag = true;
}

void sib_simple_finish(SibSimple *sib) {
	sib->drag = false;
}
