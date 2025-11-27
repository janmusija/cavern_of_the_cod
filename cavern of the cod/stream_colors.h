//
//  stream_colors.h
//  cavern of the cod
//
//  Created by jan musija on 9/22/25.
//

#ifndef stream_colors_h
#define stream_colors_h
#define ENABLE_ANSI_CODES 0
// xcode's built in terminal is annoying and doesn't use ansi codes, which is why I might want to disable this

#if ENABLE_ANSI_CODES == 'b' // black background terminal
#define BBLK "\e[1;37m"
// yes, this makes black white and such.
#define BLK "\e[0;37m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YLO "\e[0;33m"
#define BLU "\e[0;34m"
#define MGNT "\e[0;35m"
#define CYN "\e[0;36m"

#elif ENABLE_ANSI_CODES == 'w' // white background terminal

#define BBLK "\e[1;30m"
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YLO "\e[0;33m"
#define BLU "\e[0;34m"
#define MGNT "\e[0;35m"
#define CYN "\e[0;36m"

#else
#define BBLK ""
#define BLK ""
#define RED ""
#define GRN ""
#define YLO ""
#define BLU ""
#define MGNT ""
#define CYN ""

#endif

#endif /* stream_colors_h */
