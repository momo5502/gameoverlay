function blub() {
    globalThis.count = (globalThis.count || 0) + 1;

    document.querySelector(".notification.info").textContent = "FPS: " + globalThis.count;
    requestAnimationFrame(blub);
}

function run() {
    console.log("HI");
    blub();

    setTimeout(() => location.reload(), 1000);
}

window.addEventListener("DOMContentLoaded", run);
