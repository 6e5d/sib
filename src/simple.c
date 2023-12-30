#include <math.h>

#include "../include/sib.h"

static void blend_alpha(uint8_t *v, uint8_t w) {
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
	float xf = floorf(x);
	if (xf < 1.0f) { return 0; }
	if (xf >= (float)y) {
		return (y - 1);
	}
	return (uint32_t)xf;
}
static uint32_t capu(float x, uint32_t y) {
	float xc = ceilf(x);
	if (xc <= 0.0f) { return 0; }
	if (xc > (float)(y - 2)) {
		return (y - 1);
	}
	return (uint32_t)xc;
}

static void bcircle(Simpleimg()* img, float x, float y,
	uint32_t i, uint32_t j, float size, float alpha, uint8_t color[3])
{
	float dx = (float)i + 0.5f - x;
	float dy = (float)j + 0.5f - y;
	float dr = sqrtf(dx * dx + dy * dy) - size;
	if (dr >= 0.5f) { return; }
	uint8_t* p = simpleimg(offset)(img, i, j);
	memcpy(p, color, sizeof(uint8_t) * 3);
	if (fabsf(dr) < 0.5f) {
		blend_alpha(p + 3, f2u(alpha * (0.5f - dr)));
		return;
	}
	blend_alpha(p + 3, f2u(alpha));
}

static void fcircle(Simpleimg()* img, Dmgrect() *damage, float x, float y,
	float size, float alpha, uint8_t color[3]) {
	float xmaxf = ceilf(x + size);
	float xminf = floorf(x - size);
	float ymaxf = ceilf(y + size);
	float yminf = floorf(y - size);
	uint32_t xmax = capu(xmaxf, img->width);
	uint32_t xmin = capd(xminf, img->width);
	uint32_t ymax = capu(ymaxf, img->height);
	uint32_t ymin = capd(yminf, img->height);
	dmgrect(include)(damage, (int32_t)xmin, (int32_t)ymin);
	dmgrect(include)(damage, (int32_t)xmax, (int32_t)ymax);
	for (uint32_t i = xmin; i < xmax; i += 1) {
		for (uint32_t j = ymin; j < ymax; j += 1) {
			bcircle(img, x, y, i, j, size, alpha, color);
		}
	}
}

static void sib(simple_update)(void *data, float pos[3], float pps[3]) {
	Dmgrect() damage;
	dmgrect(init)(&damage);
	Sib(Simple) *sib = data;
	float x = pos[0]; float y = pos[1]; float p = pos[2];
	float px = pps[0]; float py = pps[1]; float pp = pps[2];
	float alpha1 = sib->alpha_k * pp + sib->alpha_b;
	float alpha2 = sib->alpha_k * p + sib->alpha_b;
	float size1 = sib->size_scale * (sib->size_k * pp + sib->size_b);
	float size2 = sib->size_scale * (sib->size_k * p + sib->size_b);
	float dx = x - px;
	float dy = y - py;
	float ds = size2 - size1;
	float da = alpha2 - alpha1;
	float dist = sqrtf(dx * dx + dy * dy);
	float t = 0.0;
	while (t < 1.0f) {
		float x1 = dx * t + px;
		float y1 = dy * t + py;
		float s1 = ds * t + size1;
		float a1 = da * t + alpha1;
		fcircle(sib->canvas, &damage,
			x1, y1, s1, f01cap(a1), sib->color);
		float spacing = sib->spacing * s1;
		if (spacing < sib->spacing) { spacing = sib->spacing; }
		t += spacing / dist;
	}
	dmgrect(union)(&sib->pending, &damage);
}

static void sib(simple_primary)(void *data, float dx) {
	Sib(Simple) *sib = data;
	sib->size_scale *= 1.0f + dx * 0.005f;
}

// default config
void sib(simple_config)(Sib(Simple) *sib) {
	sib->spacing = 0.25f;
	sib->alpha_k = 0.5f;
	sib->alpha_b = 0.5f;
	sib->size_k = 1.0f;
	sib->size_scale = 1.0f;
	sib->size_b = 1.0f;
	sib->color[0] = 0;
	sib->color[1] = 0;
	sib->color[2] = 0;
}

void sib(simple_config_eraser)(Sib(Simple) *sib) {
	sib->spacing = 0.25f;
	sib->alpha_k = 1.0f;
	sib->alpha_b = 1.0f;
	sib->size_k = 20.0f;
	sib->size_scale = 1.0f;
	sib->size_b = 0.0f;
	sib->color[0] = 0;
	sib->color[1] = 0;
	sib->color[2] = 0;
}

static void sib(simple_finish)(void* unused) {}

Vwdlayer(Ifdraw) sib(simple_ifdraw)(void) {
	return (Vwdlayer(Ifdraw)) {
		.end = sib(simple_finish),
		.motion = sib(simple_update),
		.primary = sib(simple_primary),
	};
}
