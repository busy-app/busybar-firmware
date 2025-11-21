#pragma once

#define M_TO_S(m)     ((m) * 60)
#define M_TO_H(m)     ((m) / 60)
#define H_TO_M(h)     ((h) * 60)
#define H_TO_S(h)     (M_TO_S(H_TO_M(h)))
#define HM_TO_M(h, m) (H_TO_M(h) + (m))
#define HM_TO_S(h, m) (M_TO_S(H_TO_M(h, m)))

#define S_TO_M(s) ((s) / 60)
#define S_TO_R(s) ((s) % 60)
#define S_TO_H(h) (S_TO_M(h) / 60)

#define S_TO_MS(s) ((s) * 1000)
#define M_TO_MS(m) (S_TO_MS(M_TO_S(m)))

#define MS_TO_S(ms) ((ms) / 1000)
