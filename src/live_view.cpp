#include "live_view.h"
#include "output.h"
#include "game.h"
#include "player.h"
#include "map.h"
#include "options.h"
#include "translations.h"
#include "vehicle.h"

#include <map>
#include <string>

const efftype_id effect_blind( "blind" );

namespace
{
constexpr int START_LINE = 1;
constexpr int START_COLUMN = 1;

} //namespace

bool live_view::is_compact() const
{
    return compact_view;
}

void live_view::set_compact( bool const value )
{
    compact_view = value;
}

void live_view::init( int const start_x, int const start_y, int const w, int const h )
{
    enabled = true;
    width   = w;
    height  = h;

    w_live_view.reset( newwin( height, width, start_y, start_x ) );

    hide();
}

void live_view::show( const int x, const int y, const visibility_variables &cache )
{
    if( !enabled || !w_live_view ) {
        return;
    }

    hide( false ); // Clear window if it's visible

    int line = START_LINE;

    // TODO: Z
    tripoint p( x, y, g->get_levz() );

    const int last_line = getmaxy( w_live_view.get() ) - START_LINE - 1;
    g->print_all_tile_info( p, *this, START_COLUMN, line, last_line, false, cache );

#if (defined TILES || defined _WIN32 || defined WINDOWS)
    // Because of the way the status UI is done, the live view window must
    // be tall enough to clear the entire height of the viewport below the
    // status bar. This hack allows the border around the live view box to
    // be drawn only as big as it needs to be, while still leaving the
    // window tall enough. Won't work for ncurses in Linux, but that doesn't
    // currently support the mouse. If and when it does, there'll need to
    // be a different code path here that works for ncurses.
    int full_height = w_live_view->height;
    if( line < w_live_view->height - 1 ) {
        w_live_view->height = std::max( line + 1, 11 );
    }
    last_height = w_live_view->height;
#endif

    draw_border( *this );
    mvwprintz( *this, 0, START_COLUMN, c_white, "< " );
    wprintz( *this, c_green, _( "Mouse View" ) );
    wprintz( *this, c_white, " >" );

#if (defined TILES || defined _WIN32 || defined WINDOWS)
    w_live_view->height = full_height;
#endif

    inuse = true;
    wrefresh( *this );
}

bool live_view::hide( bool refresh /*= true*/, bool force /*= false*/ )
{
    if( !enabled || ( !inuse && !force ) ) {
        return false;
    }

#if (defined TILES || defined _WIN32 || defined WINDOWS)
    int full_height = w_live_view->height;
    if( use_narrow_sidebar() && last_height > 0 ) {
        // When using the narrow sidebar mode, the lower part of the screen
        // is used for the message queue. Best not to obscure too much of it.
        w_live_view->height = last_height;
    }
#endif

    werase( *this );

#if (defined TILES || defined _WIN32 || defined WINDOWS)
    w_live_view->height = full_height;
#endif

    inuse = false;
    last_height = -1;
    if( refresh ) {
        wrefresh( *this );
    }

    return true;
}
