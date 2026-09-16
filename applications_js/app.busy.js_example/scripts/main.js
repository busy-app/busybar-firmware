let counter = 0;

const unbind = listen("input", handler)

function handler(event) {
    switch (event.key) {
        case "ok":
            ok_handler(event)
            break;
        case "encoder":
            encoder_handler(event)
            break;
        case "start":
            start_handler(event)
            break;
        case "back":
            unbind()
            break;
        default: break;
    }
}

function encoder_handler(event) {
    console.log("encoder dir:", event.action, "delta: ", event.delta)
    counter += event.delta
    setTimeout(displayText, 100)
}

function ok_handler(event) {
    console.log("OK:", event.action)
    if (event.action == "release") {
        counter = 0
        setTimeout(displayText, 100)
    }
}

function start_handler(event) {
    console.log("start", event.action)
    if (event.action == "release") {
        counter = 0
        setTimeout(displayText, 100)
    }
}

function displayText() {
    const request = new Request(
        "http://127.0.0.1/api/display/draw",
        {
            method: "POST",
            body: JSON.stringify({
                "application_name": "app.busy.js_example",
                "elements": [
                    {
                        "id": "0",
                        "type": "text",
                        "x": 72 / 2,
                        "y": 16 / 2,
                        "align": "center",
                        "text": "JS counter: " + counter,
                        "font": "small"
                    }
                ]
            })
        });

    fetch(request);
}

displayText();