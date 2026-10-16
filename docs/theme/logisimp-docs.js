// Local initialization; upstream Doxygen Awesome scripts remain unmodified.
DoxygenAwesomeDarkModeToggle.init();
DoxygenAwesomeParagraphLink.init();

document.addEventListener("DOMContentLoaded", () => {
    // Sidebar-only CSS positions the title, search, and content using this
    // variable. Follow Doxygen's actual width, including its drag-handle padding.
    const sideNav = document.getElementById("side-nav");
    if (sideNav) {
        const syncSidebarWidth = () => {
            if (window.matchMedia("(min-width: 768px)").matches) {
                document.documentElement.style.setProperty(
                    "--side-nav-fixed-width",
                    `${sideNav.getBoundingClientRect().width}px`
                );
            }
        };
        new ResizeObserver(syncSidebarWidth).observe(sideNav);
        syncSidebarWidth();
    }

    // Newer Doxygen initializes its own clipboard buttons earlier on this
    // event. Keep those when present, instead of adding a second copy button.
    if (!document.querySelector(".fragment .clipboard")) {
        DoxygenAwesomeFragmentCopyButton.init();
    }

    // The left tree provides navigation. logisimp-layout.css hides both the
    // native page outline and classic TOC, so no extra TOC widget is initialized.
});
