# Bundled documentation theme

`doxygen-awesome.css`, `doxygen-awesome-sidebar-only.css`,
`doxygen-awesome-sidebar-only-darkmode-toggle.css`, and the four
`doxygen-awesome-*.js` files are unmodified copies from
[Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css), release
**v2.5.0**, commit `46483f1e5a70ffb9ecd3b82d0a1cd1b24edf13da`.

The upstream MIT license is preserved in [LICENSE](LICENSE).

The root `Doxyfile` uses the sidebar-only theme with a navigation tree. It keeps
`GENERATE_TREEVIEW = YES`, `DISABLE_INDEX = NO`, `FULL_SIDEBAR = NO`, and
`HTML_COLORSTYLE = LIGHT`. `TREEVIEW_WIDTH = 335` sets the initial sidebar width.
The title, search field, dark-mode toggle, and navigation live on the left;
there is no top index bar or right-hand contents panel.
The sidebar-only CSS hides desktop tabs; `DISABLE_INDEX = NO` retains the
responsive navigation menu on small screens, as upstream requires.

The bundled JavaScript helpers provide a light/dark toggle, fragment copy
buttons, and heading permalinks. They are
included by `docs/header.html`; `logisimp-docs.js` is our local initialization
and compatibility code, separate from the unmodified upstream assets.
Local layout overrides restore Doxygen's resize handle and allow widths from
240px to half the viewport. A ResizeObserver in `logisimp-docs.js` keeps the
sidebar theme's width variable in sync with Doxygen's actual sidebar width, so
the title, search field, navigation footer, and content stay aligned. Doxygen's
resize script handles dragging and saving the width. Mobile navigation retains
the upstream layout.

The interactive TOC helper remains bundled but is not initialized in this layout.

`logisimp-function-preview.js` is a separate local extension for same-page
function links in class member declaration lists. After a 250ms hover, or on
keyboard focus, it clones the function's signature and documentation into a
floating panel. Examples and implementation/source code remain in the full
details opened by the original link. Inherited members and cross-page links are
skipped. The extension reads the current DOM without fetching documentation,
so it works with local `file://` pages and needs no additional library.

It matches function-group declaration rows and their fragment IDs
(`memitem:...` or `r_...`) to the associated `.memproto` and `.memdoc` only.
A cached template holds each member's signature and prose once, without
repeating the declaration brief. Clones omit examples, source/code blocks,
graphs, and IDs; cloned links become plain text for a read-only tooltip.
The extension initializes itself when the document is ready, separately from
the Awesome/sidebar initializer in `logisimp-docs.js`.
Member matching and preview content have been checked against generated markup
from Doxygen 1.9.8 and 1.18.0. Recheck other Doxygen output with the maintenance
steps in `docs/README.md`.

Preview colors, borders, signature wrapping, and viewport bounds are defined
in `logisimp-layout.css`, using Awesome's existing light/dark theme variables.
The tooltip describes its focused function link with `aria-describedby`.
Escape dismisses it; Up/Down, Page Up/Down, and Home/End scroll while the
function link is focused. The panel also accepts pointer scrolling, and Tab
retains its normal focus order. Keep the extension's script registration in
the custom header and its asset entry in `Doxyfile` when updating the setup.
See the function-preview maintenance checks in `docs/README.md`; regenerate
with `make d`, then use `make do` to open the site when desired.

The header is based on the shared Doxygen 1.9.6+ template fields, checked
against the 1.9.8 and 1.18 templates. Navigation/search startup remains generated
by Doxygen via its placeholders. A synchronous, parse-time version guard loads
Doxygen's own `jquery.js` only before 1.17, `clipboard.js` from 1.10 onward,
and `codefolding.js` on 1.17+ so native source-folding initialization can run.
There are no external scripts or version-specific Doxyfile settings.

The initializer preserves native `.fragment .clipboard` buttons where present;
Awesome supplies copy buttons on older output. `logisimp-layout.css` hides both
the native `#page-nav` outline and the classic inline contents box, and removes
the right-hand grid column reserved by newer Doxygen releases. This keeps the
layout compatible with older generators without adding unsupported
`PAGE_OUTLINE_PANEL` settings. Dark mode and heading permalinks
use Awesome on every version. Clipboard buttons require browser clipboard
support, typically provided by localhost or HTTPS.

To update the theme, copy all three upstream stylesheets, all four helper scripts, and license
from a chosen upstream release. Update this version record, review upstream
extension installation notes, and regenerate the docs to check rendering. After
upgrading Doxygen, compare `doxygen -w html` output with `docs/header.html` as
described in `docs/README.md`; retain the extension includes and compatibility
guard when adjusting the header.
