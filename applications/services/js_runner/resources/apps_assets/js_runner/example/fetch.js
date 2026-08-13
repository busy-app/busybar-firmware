async function async_chunks() {
    console.info("Async chunks");
    const url = "https://qdiv.dev";
    const response = await fetch(url);
    for await (const chunk of response.body) {
        console.log("Chunk size:", chunk.length);
    }
}

async function json_body() {
    console.info("JSON body");
    const url = "http://10.0.4.20/api/display/brightness";
    const response = await fetch(url);
    const data = await response.json();
    console.log("Brightness value:", data.value);
}

async function post() {
    console.info("POST request");
    const request = new Request(
        "http://10.0.4.20/api/display/draw",
        {
            method: "POST",
            body: `{
                      "application_name": "my_app",
                      "led_notification_color": "#FF0000FF",
                      "elements": [
                        {
                          "id": "0",
                          "timeout": 10,
                          "align": "center",
                          "x": 36,
                          "y": 10,
                          "type": "text",
                          "text": "Hello, World! Long text",
                          "font": "normal",
                          "color": "#FFFFFFFF",
                          "width": 72,
                          "scroll_rate": 1000,
                          "scroll_start_delay": 1000,
                          "scroll_repeat_delay": 2500,
                          "display": "front"
                        },
                        {
                          "id": "1",
                          "timeout": 6,
                          "align": "top_mid",
                          "x": 36,
                          "y": 0,
                          "type": "text",
                          "text": "top_mid",
                          "font": "small",
                          "color": "#AAFF00FF",
                          "display": "front"
                        }
                      ]
                    }`,
            // headers: {"Connection": "close"},
        });
    const response = await fetch(request);
    const result = await response.json();
    console.log("Result:", result.result);
}

async function run_all() {
    await post();
    await async_chunks();
    await json_body();
}

run_all().then(() => {
    console.info("All done");
}).catch((e) => {
    console.error("Error:", e);
})
