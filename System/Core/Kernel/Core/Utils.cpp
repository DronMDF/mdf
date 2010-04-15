//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/CoreLocal.h"

namespace Core {

// -----------------------------------------------------------------------------
// Для генерации последовательности будем использовать хороший алгоритм -
// вихрь мерсенна - http://ru.wikipedia.org/wiki/Вихрь_Мерсенна

// TODO: Это все надо еще оптимизировать... написано как-то криво - списывал.

class RandomGenerator
{
protected:
	enum { N = 624,  M = 397 };

	int sidx;
	uint32_t state[N];

	void reload (void);

	uint32_t mixBits(uint32_t u, uint32_t v) const
	{
		return (u & 0x80000000U) | (v & 0x7fffffffU);
	}

	uint32_t twist(uint32_t m, uint32_t s0, uint32_t s1) const
	{
		const uint32_t y = mixBits (s0, s1);
		return m ^ (y >> 1) ^ ((y & 1) ? 0x9908b0dfU : 0U);
	}

	uint32_t xorBits(unsigned int i) const
	{
		return state[i] ^ (state[i] >> 30);
	}

public:
	void entropy ();

	RandomGenerator ()
		: sidx(0)
	{
		entropy ();
	}

	//explicit RandomGenerator (const uint32_t *key, size_t keylength);

	uint32_t get();
};

void RandomGenerator::entropy ()
{
	// TODO: Проинициализировать каким нибудь значением.
	state[0] = 0xDEADBEAF;
	for (unsigned int i = 1; i < N; i++)
		state[i] = 1812433253U * xorBits(i - 1) + i;
	reload ();
}

void RandomGenerator::reload (void)
{
	uint32_t *p = state;
	for (int i = N - M; i--; ++p)
		*p = twist (p[M], p[0], p[1]);

	for (int i = M; --i; ++p)
		*p = twist (p[M-N], p[0], p[1]);

	*p = twist(p[M-N], p[0], state[0]);

	sidx = 0;
}

// RandomGenerator::RandomGenerator (const uint32_t *key, size_t keylength)
// {
// 	init (19650218UL);
//
// 	int i = 1, j = 0;
//
// 	for (int k = max ((size_t)N, keylength); k > 0; k--) {
// 		state[i] = (state[i] ^ (xorBits(i - 1) * 1664525UL)) + key[j] + j;
// 		i++; j++;
//
// 		if (i >= N) {
// 			state[0] = state[N-1];
// 			i = 1;
// 		}
//
// 		j %= keylength;
// 	}
//
// 	for (int k = N - 1; k; k--) {
// 		state[i] = (state[i] ^ (xorBits(i - 1) * 1566083941UL)) - i;
// 		i++;
// 		if (i >= N) {
// 			state[0] = state[N - 1];
// 			i = 1;
// 		}
// 	}
//
// 	state[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
// 	reload ();
// }

uint32_t RandomGenerator::get()
{
	if (sidx == N)
		reload();

	uint32_t y = state[sidx++];

	// Tempering
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680U;
	y ^= (y << 15) & 0xefc60000U;
	y ^= (y >> 18);

	return y;
}

// -----------------------------------------------------------------------------
static
RandomGenerator *rnd = 0;

void __init__ InitUtils(void)
{
	// Генератор случайных чисел инициализируем содержимым биоса.
	//rnd = new RandomGenerator ((uint32_t *)0xf0000,
	//				0x10000 / sizeof (uint32_t));

	rnd = new RandomGenerator;
}

} // namespace Core

// Эту функцию можно вызывать и до инициализации коры, она вполне самодостаточна.
// Правда рандомизации от нее будет не много ну и чтож с того?

extern "C"
uint32_t CoreRandom (void)
{
	if (Core::rnd != 0)
		return Core::rnd->get ();

	return 0xDEADBEAF;	// Default random! :)
}
