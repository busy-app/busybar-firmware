/**
 * @file overlap_fader.h
 * @brief Gradient fader widget for creating fade effects at widget edges.
 */
#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/** OverlapFader opaque structure. */
typedef struct OverlapFader OverlapFader;

/** Side to position the fader relative to a target widget. */
typedef enum {
    OverlapFaderSideLeft, /**< Position left of target, fade: transparent->opaque (L->R) */
    OverlapFaderSideRight, /**< Position right of target, fade: opaque->transparent (L->R) */
    OverlapFaderSideTop, /**< Position above target, fade: transparent->opaque (T->B) */
    OverlapFaderSideBottom, /**< Position below target, fade: opaque->transparent (T->B) */

    OverlapFaderSidesCount, /**< Number of sides */
} OverlapFaderSide;

/**
 * @brief Create a new OverlapFader instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created OverlapFader instance
 */
OverlapFader* overlap_fader_alloc(Widget* parent);

/**
 * @brief Delete an OverlapFader instance.
 *
 * @param[in,out] instance pointer to the OverlapFader instance to be deleted
 */
void overlap_fader_free(OverlapFader* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * @param[in,out] instance pointer to the OverlapFader instance to be queried
 * @returns pointer to the base class instance
 */
Widget* overlap_fader_get_base(OverlapFader* instance);

/**
 * @brief Position the fader relative to a target widget.
 *
 * Also configures gradient direction and opacity based on the chosen side.
 *
 * @param[in,out] instance pointer to the OverlapFader instance
 * @param[in] target pointer to the target Widget to align to
 * @param[in] side side to position the fader on
 */
void overlap_fader_align_to(OverlapFader* instance, Widget* target, OverlapFaderSide side);

#ifdef __cplusplus
}
#endif
