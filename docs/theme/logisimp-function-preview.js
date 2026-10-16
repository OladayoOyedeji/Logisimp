/* Local, read-only previews of Doxygen class member documentation. */
(function () {
    "use strict";

    document.addEventListener("DOMContentLoaded", function () {
        const methodGroups = new Set([
            "pub-methods", "pub-static-methods", "pro-methods", "pro-static-methods",
            "pri-methods", "pri-static-methods", "pub-slots", "pro-slots", "pri-slots",
            "signals", "friends", "related"
        ]);
        function isMethodGroup(heading) {
            return Array.from(heading.querySelectorAll("[id], a[name]")).some(function (anchor) {
                const group = anchor.id || anchor.getAttribute("name");
                return methodGroups.has(group.replace(/^header-/, ""));
            });
        }
        if (!Array.from(document.querySelectorAll("table.memberdecls tr.heading")).some(isMethodGroup)) {
            return;
        }
        const templates = new Map();
        const titles = new Map();
        const panel = document.createElement("div");
        panel.className = "logisimp-function-preview";
        panel.id = "logisimp-function-preview";
        while (document.getElementById(panel.id)) {
            panel.id += "-local";
        }
        panel.setAttribute("role", "tooltip");
        panel.hidden = true;
        document.body.appendChild(panel);

        let activeLink = null;
        let pointerLink = null;
        let focusLink = null;
        let pendingLink = null;
        let dismissedLink = null;
        let pointerInPanel = false;
        let touchInput = false;
        let openTimer = null;
        let closeTimer = null;
        let positionFrame = null;

        function isMethodRow(row) {
            for (let heading = row.previousElementSibling; heading; heading = heading.previousElementSibling) {
                if (heading.classList.contains("heading")) {
                    return isMethodGroup(heading);
                }
            }
            return false;
        }

        function memberId(link) {
            const row = link.closest("tr");
            if (!row || !link.closest("td.memItemRight") || !row.closest("table.memberdecls") || !isMethodRow(row)) {
                return null;
            }
            let destination;
            let id;
            try {
                destination = new URL(link.getAttribute("href"), document.baseURI);
                id = decodeURIComponent(destination.hash.slice(1));
            } catch (error) {
                return null;
            }
            const page = new URL(window.location.href);
            page.hash = "";
            destination.hash = "";
            // Inherited members on another page retain normal navigation. No fetch is needed.
            if (!id || destination.href !== page.href) {
                return null;
            }
            const memberClass = Array.from(row.classList).find(function (name) {
                return name.startsWith("memitem:");
            });
            const rowId = row.id.startsWith("r_") ? row.id.slice(2) : memberClass && memberClass.slice(8);
            return rowId === id ? id : null;
        }

        function detailFor(id) {
            const anchor = document.getElementById(id) || document.getElementsByName(id)[0];
            if (!anchor) {
                return null;
            }
            let detail = anchor.closest(".memitem");
            if (!detail) {
                let next = (anchor.closest(".memtitle") || anchor).nextElementSibling;
                if (next && next.classList.contains("memtitle")) {
                    next = next.nextElementSibling;
                }
                // Do not search past this member into a later member's documentation.
                detail = next && next.classList.contains("memitem") ? next : null;
            }
            const signature = detail && detail.querySelector(".memproto");
            const name = signature && signature.querySelector("table.memname");
            return name && /\([\s\S]*\)/.test(name.textContent) ? detail : null;
        }

        function removeExamples(body) {
            body.querySelectorAll("dl.section").forEach(function (section) {
                const title = section.querySelector("dt");
                if (title && /^examples?\b/i.test(title.textContent.trim())) {
                    section.remove();
                }
            });
            // Doxygen can also render an example as a Markdown heading.
            body.querySelectorAll("h1, h2, h3, h4, h5, h6").forEach(function (heading) {
                if (!/^examples?\b/i.test(heading.textContent.trim())) {
                    return;
                }
                const level = Number(heading.tagName.slice(1));
                let next = heading.nextElementSibling;
                while (next && !(/^H[1-6]$/.test(next.tagName) && Number(next.tagName.slice(1)) <= level)) {
                    const following = next.nextElementSibling;
                    next.remove();
                    next = following;
                }
                heading.remove();
            });
            body.querySelectorAll(".fragment, pre, .doxygen-awesome-fragment-wrapper, doxygen-awesome-fragment-copy-button, .clipboard, .ttc, .dynheader, .dyncontent, .dotgraph, .definition, .reference, .referencedby").forEach(function (node) {
                node.remove();
            });
        }

        function cleanClone(original) {
            const clone = original.cloneNode(true);
            clone.querySelectorAll("script, style, link, base, meta, iframe, object, embed, form, input, button, select, textarea, img, picture, video, audio, source, svg, canvas").forEach(function (node) {
                node.remove();
            });
            // A tooltip is read-only: leave navigation and examples on the original page.
            clone.querySelectorAll("a").forEach(function (anchor) {
                const text = document.createElement("span");
                text.className = anchor.className;
                while (anchor.firstChild) {
                    text.appendChild(anchor.firstChild);
                }
                anchor.replaceWith(text);
            });
            [clone].concat(Array.from(clone.querySelectorAll("*"))).forEach(function (node) {
                Array.from(node.attributes).forEach(function (attribute) {
                    if (/^(id|name|style|title|tabindex|autofocus|contenteditable)$/i.test(attribute.name) || /^on/i.test(attribute.name)) {
                        node.removeAttribute(attribute.name);
                    }
                });
            });
            return clone;
        }

        function templateFor(link) {
            const id = memberId(link);
            if (!id) {
                return null;
            }
            if (!templates.has(id)) {
                const detail = detailFor(id);
                if (!detail) {
                    templates.set(id, null);
                } else {
                    const content = document.createDocumentFragment();
                    const signature = cleanClone(detail.querySelector(".memproto"));
                    signature.classList.add("logisimp-function-preview__signature");
                    content.appendChild(signature);
                    const originalBody = detail.querySelector(".memdoc");
                    if (originalBody) {
                        const body = originalBody.cloneNode(true);
                        removeExamples(body);
                        const cleanBody = cleanClone(body);
                        cleanBody.classList.add("logisimp-function-preview__body");
                        content.appendChild(cleanBody);
                    }
                    const hint = document.createElement("p");
                    hint.className = "logisimp-function-preview__hint";
                    hint.textContent = "Click the function for full documentation and examples.";
                    content.appendChild(hint);
                    templates.set(id, content);
                }
            }
            return templates.get(id);
        }

        function functionLink(target) {
            const link = target instanceof Element && target.closest("a.el[href]");
            return link && templateFor(link) ? link : null;
        }

        function suppressTitle(link) {
            if (link.hasAttribute("title") && !titles.has(link)) {
                titles.set(link, link.getAttribute("title"));
                link.removeAttribute("title");
            }
        }

        function restoreTitles() {
            titles.forEach(function (title, link) {
                if (link !== pointerLink && link !== focusLink) {
                    link.setAttribute("title", title);
                    titles.delete(link);
                }
            });
        }

        function cancelOpen() {
            window.clearTimeout(openTimer);
            openTimer = null;
            pendingLink = null;
        }

        function hide() {
            window.clearTimeout(closeTimer);
            if (activeLink) {
                const tokens = (activeLink.getAttribute("aria-describedby") || "").split(/\s+/).filter(function (token) {
                    return token && token !== panel.id;
                });
                if (tokens.length) {
                    activeLink.setAttribute("aria-describedby", tokens.join(" "));
                } else {
                    activeLink.removeAttribute("aria-describedby");
                }
            }
            activeLink = null;
            pointerInPanel = false;
            panel.hidden = true;
        }

        function position() {
            if (!activeLink || panel.hidden) {
                return;
            }
            const margin = 12;
            const gap = 8;
            const width = document.documentElement.clientWidth;
            const height = document.documentElement.clientHeight;
            const rect = activeLink.getBoundingClientRect();
            if (rect.bottom <= 0 || rect.top >= height || rect.right <= 0 || rect.left >= width) {
                hide();
                return;
            }
            panel.style.maxHeight = Math.max(0, height - margin * 2) + "px";
            const naturalHeight = Math.min(panel.scrollHeight + 2, 512);
            const below = Math.max(0, height - rect.bottom - gap - margin);
            const above = Math.max(0, rect.top - gap - margin);
            const useBelow = below >= Math.min(naturalHeight, 320) || below >= above;
            panel.style.maxHeight = Math.min(512, useBelow ? below : above) + "px";
            const bounds = panel.getBoundingClientRect();
            panel.style.left = Math.max(margin, Math.min(rect.left, width - bounds.width - margin)) + "px";
            const top = useBelow ? rect.bottom + gap : rect.top - gap - bounds.height;
            panel.style.top = Math.max(margin, Math.min(top, height - bounds.height - margin)) + "px";
        }

        function show(link) {
            if (link === dismissedLink) {
                return;
            }
            const template = templateFor(link);
            if (!template) {
                return;
            }
            cancelOpen();
            hide();
            activeLink = link;
            panel.replaceChildren(template.cloneNode(true));
            const tokens = (link.getAttribute("aria-describedby") || "").split(/\s+/).filter(Boolean);
            if (!tokens.includes(panel.id)) {
                tokens.push(panel.id);
            }
            link.setAttribute("aria-describedby", tokens.join(" "));
            panel.hidden = false;
            panel.scrollTop = 0;
            position();
        }

        function closeIfUnused() {
            window.clearTimeout(closeTimer);
            closeTimer = window.setTimeout(function () {
                if (activeLink !== pointerLink && activeLink !== focusLink && !pointerInPanel) {
                    hide();
                }
            }, 180);
        }

        document.addEventListener("pointerover", function (event) {
            if (event.pointerType === "touch") {
                return;
            }
            const link = functionLink(event.target);
            if (!link || link.contains(event.relatedTarget)) {
                return;
            }
            pointerLink = link;
            dismissedLink = null;
            suppressTitle(link);
            window.clearTimeout(closeTimer);
            cancelOpen();
            if (activeLink !== link) {
                pendingLink = link;
                openTimer = window.setTimeout(function () { show(link); }, 250);
            }
        });

        document.addEventListener("pointerout", function (event) {
            if (!pointerLink || !pointerLink.contains(event.target) || pointerLink.contains(event.relatedTarget)) {
                return;
            }
            if (pendingLink === pointerLink) {
                cancelOpen();
            }
            pointerLink = null;
            restoreTitles();
            closeIfUnused();
        });

        panel.addEventListener("pointerenter", function () {
            pointerInPanel = true;
            window.clearTimeout(closeTimer);
        });
        panel.addEventListener("pointerleave", function () {
            pointerInPanel = false;
            closeIfUnused();
        });
        document.addEventListener("pointerdown", function (event) {
            touchInput = event.pointerType === "touch";
            if (touchInput) {
                cancelOpen();
                hide();
            }
        });
        document.addEventListener("focusin", function (event) {
            const link = functionLink(event.target);
            if (touchInput || !link) {
                return;
            }
            focusLink = link;
            dismissedLink = null;
            suppressTitle(link);
            show(link);
        });
        document.addEventListener("focusout", function (event) {
            if (event.target !== focusLink) {
                return;
            }
            focusLink = null;
            cancelOpen();
            restoreTitles();
            closeIfUnused();
        });
        document.addEventListener("keydown", function (event) {
            touchInput = false;
            if (event.key === "Escape") {
                dismissedLink = activeLink || pendingLink || focusLink || pointerLink;
                cancelOpen();
                hide();
            } else if (activeLink && event.target === activeLink && !event.altKey && !event.ctrlKey && !event.metaKey) {
                const scroll = {
                    ArrowDown: 40, ArrowUp: -40,
                    PageDown: panel.clientHeight * 0.8, PageUp: -panel.clientHeight * 0.8,
                    Home: -panel.scrollHeight, End: panel.scrollHeight
                };
                if (Object.prototype.hasOwnProperty.call(scroll, event.key)) {
                    panel.scrollTop += scroll[event.key];
                    event.preventDefault();
                }
            }
        });
        document.addEventListener("click", function (event) {
            const link = functionLink(event.target);
            if (link) {
                dismissedLink = link;
                cancelOpen();
                hide();
                // The original href, default click, and browser history remain untouched.
            }
        });
        function schedulePosition(event) {
            if ((event.target instanceof Node && panel.contains(event.target)) || positionFrame !== null) {
                return;
            }
            positionFrame = window.requestAnimationFrame(function () {
                positionFrame = null;
                position();
            });
        }
        document.addEventListener("scroll", schedulePosition, { capture: true, passive: true });
        window.addEventListener("resize", schedulePosition);
        window.addEventListener("pagehide", function () {
            cancelOpen();
            hide();
            pointerLink = null;
            focusLink = null;
            restoreTitles();
        });
    });
}());
