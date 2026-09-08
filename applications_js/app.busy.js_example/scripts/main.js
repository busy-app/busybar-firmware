let counter = 0;

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
                        "text": "JS Example: " + counter,
                        "font": "small"
                    }
                ]
            })
        });

    fetch(request);
    counter++;
}

displayText();
setInterval(displayText, 1000);
