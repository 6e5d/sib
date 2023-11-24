#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "../../simpleimg/include/simpleimg.h"
#include "../../dmgrect/include/dmgrect.h"
#include "../include/simple.h"

static inline void blend_alpha(uint8_t *v, uint8_t w) {
	if (w > *v) {*v = w;}
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

static uint32_t capd(float x, uint32_t y) {
	if (x < 0) { return 0; }
	if ((uint32_t)x >= y) {
		return (y - 1);
	}
	return (uint32_t)floorf(x);
}
static uint32_t capu(float x, uint32_t y) {
	if (x < 0) { return 0; }
	if ((uint32_t)x >= y) {
		return (y - 1);
	}
	return (uint32_t)ceilf(x);
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

static void fcircle(Simpleimg* img, Dmgrect *damage, float x, float y,
	float size, float alpha, uint8_t color[3]) {
	float xmaxf = ceilf(x + size);
	float xminf = floorf(x - size);
	float ymaxf = ceilf(y + size);
	float yminf = floorf(y - size);
	uint32_t xmax = capu(xmaxf, img->width);
	uint32_t xmin = capd(xminf, img->width);
	uint32_t ymax = capu(ymaxf, img->height);
	uint32_t ymin = capd(yminf, img->height);
	dmgrect_include(damage, (int32_t)xmin, (int32_t)ymin);
	dmgrect_include(damage, (int32_t)xmax, (int32_t)ymax);
	for (uint32_t i = xmin; i < xmax; i += 1) {
		for (uint32_t j = ymin; j < ymax; j += 1) {
			bcircle(img, x, y, i, j, size, alpha, color);
		}
	}
}

static void sib_simple_update2(SibSimple *sib, Simpleimg *img,
	Dmgrect *damage, float x2, float y2, float p)
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
		fcircle(img, damage, x, y, s, f01cap(a), sib->color);
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
}

void sib_simple_update(SibSimple *sib, Simpleimg *img,
	Dmgrect *damage, float x, float y, float p)
{
	if (sib->drag) {
		sib_simple_update2(sib, img, damage, x, y, p);
	}
	sib->px = x;
	sib->py = y;
	sib->pp = p;
	sib->drag = true;
}

void sib_simple_finish(SibSimple *sib) {
	sib->drag = false;
}
