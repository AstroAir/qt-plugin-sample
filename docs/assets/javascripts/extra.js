/* Extra JavaScript for QtPlugin documentation */

document.addEventListener('DOMContentLoaded', function () {
    // Add copy functionality to code blocks
    addCopyButtons();

    // Initialize tooltips
    initializeTooltips();

    // Add version badges
    addVersionBadges();

    // Initialize search enhancements
    enhanceSearch();

    // Add keyboard shortcuts
    addKeyboardShortcuts();
});

/**
 * Add copy buttons to code blocks
 */
function addCopyButtons() {
    const codeBlocks = document.querySelectorAll('pre code');

    codeBlocks.forEach(function (codeBlock) {
        const pre = codeBlock.parentNode;
        const button = document.createElement('button');

        button.className = 'md-clipboard';
        button.title = 'Copy to clipboard';
        button.innerHTML = '📋';

        button.addEventListener('click', function () {
            navigator.clipboard.writeText(codeBlock.textContent).then(function () {
                button.innerHTML = '✅';
                button.title = 'Copied!';

                setTimeout(function () {
                    button.innerHTML = '📋';
                    button.title = 'Copy to clipboard';
                }, 2000);
            });
        });

        pre.style.position = 'relative';
        pre.appendChild(button);
    });
}

/**
 * Initialize tooltips for API elements
 */
function initializeTooltips() {
    const apiElements = document.querySelectorAll('[data-tooltip]');

    apiElements.forEach(function (element) {
        element.addEventListener('mouseenter', function () {
            showTooltip(element, element.getAttribute('data-tooltip'));
        });

        element.addEventListener('mouseleave', function () {
            hideTooltip();
        });
    });
}

/**
 * Show tooltip
 */
function showTooltip(element, text) {
    const tooltip = document.createElement('div');
    tooltip.className = 'custom-tooltip';
    tooltip.textContent = text;

    document.body.appendChild(tooltip);

    const rect = element.getBoundingClientRect();
    tooltip.style.left = rect.left + (rect.width / 2) - (tooltip.offsetWidth / 2) + 'px';
    tooltip.style.top = rect.top - tooltip.offsetHeight - 10 + 'px';
}

/**
 * Hide tooltip
 */
function hideTooltip() {
    const tooltip = document.querySelector('.custom-tooltip');
    if (tooltip) {
        tooltip.remove();
    }
}

/**
 * Add version badges to API elements
 */
function addVersionBadges() {
    const versionElements = document.querySelectorAll('[data-version]');

    versionElements.forEach(function (element) {
        const version = element.getAttribute('data-version');
        const badge = document.createElement('span');
        badge.className = 'version-badge';
        badge.textContent = 'v' + version;

        element.appendChild(badge);
    });
}

/**
 * Enhance search functionality
 */
function enhanceSearch() {
    const searchInput = document.querySelector('[data-md-component="search-query"]');

    if (searchInput) {
        // Add search suggestions
        searchInput.addEventListener('input', function () {
            const query = this.value.toLowerCase();

            if (query.length > 2) {
                addSearchSuggestions(query);
            }
        });
    }
}

/**
 * Add search suggestions
 */
function addSearchSuggestions(query) {
    const suggestions = [
        'plugin interface',
        'plugin manager',
        'plugin loader',
        'security validation',
        'error handling',
        'lifecycle management',
        'configuration',
        'hot reloading',
        'inter-plugin communication'
    ];

    const matches = suggestions.filter(s => s.includes(query));

    // Implementation would depend on MkDocs search structure
    console.log('Search suggestions:', matches);
}

/**
 * Add keyboard shortcuts
 */
function addKeyboardShortcuts() {
    document.addEventListener('keydown', function (e) {
        // Ctrl/Cmd + K for search
        if ((e.ctrlKey || e.metaKey) && e.key === 'k') {
            e.preventDefault();
            const searchInput = document.querySelector('[data-md-component="search-query"]');
            if (searchInput) {
                searchInput.focus();
            }
        }

        // Escape to close search
        if (e.key === 'Escape') {
            const searchInput = document.querySelector('[data-md-component="search-query"]');
            if (searchInput && document.activeElement === searchInput) {
                searchInput.blur();
            }
        }

        // G + H for home
        if (e.key === 'g' && !e.ctrlKey && !e.metaKey) {
            setTimeout(function () {
                document.addEventListener('keydown', function homeHandler(e2) {
                    if (e2.key === 'h') {
                        window.location.href = '/';
                        document.removeEventListener('keydown', homeHandler);
                    }
                }, { once: true });
            }, 100);
        }
    });
}

/**
 * Add interactive elements for plugin examples
 */
function addInteractiveElements() {
    // Add expandable code sections
    const expandableCode = document.querySelectorAll('.expandable-code');

    expandableCode.forEach(function (element) {
        const header = element.querySelector('.code-header');
        const content = element.querySelector('.code-content');

        if (header && content) {
            header.addEventListener('click', function () {
                content.style.display = content.style.display === 'none' ? 'block' : 'none';
                header.classList.toggle('expanded');
            });
        }
    });
}

/**
 * Initialize plugin capability filters
 */
function initializeCapabilityFilters() {
    const filterButtons = document.querySelectorAll('.capability-filter');
    const pluginItems = document.querySelectorAll('.plugin-item');

    filterButtons.forEach(function (button) {
        button.addEventListener('click', function () {
            const capability = this.getAttribute('data-capability');

            // Toggle active state
            this.classList.toggle('active');

            // Filter plugin items
            pluginItems.forEach(function (item) {
                const itemCapabilities = item.getAttribute('data-capabilities').split(',');
                const activeFilters = Array.from(document.querySelectorAll('.capability-filter.active'))
                    .map(btn => btn.getAttribute('data-capability'));

                if (activeFilters.length === 0 || activeFilters.some(filter => itemCapabilities.includes(filter))) {
                    item.style.display = 'block';
                } else {
                    item.style.display = 'none';
                }
            });
        });
    });
}

/**
 * Add smooth scrolling for anchor links
 */
function addSmoothScrolling() {
    const anchorLinks = document.querySelectorAll('a[href^="#"]');

    anchorLinks.forEach(function (link) {
        link.addEventListener('click', function (e) {
            e.preventDefault();

            const targetId = this.getAttribute('href').substring(1);
            const targetElement = document.getElementById(targetId);

            if (targetElement) {
                targetElement.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        });
    });
}

// Initialize additional features when DOM is ready
document.addEventListener('DOMContentLoaded', function () {
    addInteractiveElements();
    initializeCapabilityFilters();
    addSmoothScrolling();
});
