#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "../../simpleimg/include/simpleimg.h"
#include "../include/simple.h"

static void blend_alpha(uint8_t *v, uint8_t w) {
	if (w > *v) {*v = w;}
}

static float f01cap(float x) {
	if (x < 0.0f) { return 0.0; }
	if (x > 1.0f) { return 1.0; }
	return x;
}

static uint32_t cap(float x, uint32_t y) {
	if (x < 0) { return 0; }
	if ((uint32_t)x >= y) {
		return (y - 1);
	}
	return (uint32_t)x;
}

static void fcircle(Simpleimg* img, float x, float y, float size, float alpha) {
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
			float dx = (float)i + 0.5f - x;
			float dy = (float)j + 0.5f - y;
			float dr = sqrtf(dx * dx + dy * dy) - size;
			if (dr >= 0.5f) { continue; }
			uint8_t* p = simpleimg_offset(img, i, j);
			*(p + 0) = 0;
			*(p + 1) = 0;
			*(p + 2) = 0;
			if (fabsf(dr) < 0.5f) {
				blend_alpha(p + 3,
					(uint8_t)(256.f * (0.5f - dr)));
				continue;
			}
			blend_alpha(p + 3, 255);
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
		fcircle(img, x, y, s, f01cap(a));
		float spacing = sib->spacing * s;
		if (spacing < sib->spacing) { spacing = sib->spacing; }
		t += spacing / dist;
	}
}

// default config
void sib_simple_config(SibSimple *sib) {
	sib->spacing = 0.25;
	sib->alpha_k = 0.0;
	sib->alpha_b = 1.0;
	sib->size_k = 5.0;
	sib->size_b = 0.0;
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
