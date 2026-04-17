#ifndef SL_BTCTRL_ANCHOR_SELECTION_CONFIG_H
#define SL_BTCTRL_ANCHOR_SELECTION_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Bluetooth Controller Anchor Selection Configuration

#define SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EVEN         0
#define SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EMPTY_CENTER 1

// <o SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM> Controller's anchor selection algorithm
//   <SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EVEN=> Even - Best for single central multi-peripheral cases, by maximizing airtime and minimizing collisions.
//   <SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EMPTY_CENTER=> Empty center - Best for ESL PAwR advertiser use cases, especially when PAwR subevents and connections have the same interval, it relies on procedures runtime.
// <i> Default: SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EVEN
#ifndef SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM
#define SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EVEN
#endif // SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM

// </h> Bluetooth Controller Anchor Selection Configuration

// <<< end of configuration section >>>

#endif // SL_BTCTRL_ANCHOR_SELECTION_CONFIG_H
