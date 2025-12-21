(function() {
    console.log('=== DEBUG LAYOUT ===');
    console.log('Window width:', window.innerWidth);
    console.log('Window height:', window.innerHeight);
    console.log('Device pixel ratio:', window.devicePixelRatio);
    
    document.querySelectorAll('*').forEach(el => {
        const rect = el.getBoundingClientRect();
        if(rect.left < 0 || rect.right > window.innerWidth) {
            console.warn('Element outside viewport:', el);
            console.warn('Rect:', rect);
            el.style.outline = '2px solid red';
        }
    });
})();
