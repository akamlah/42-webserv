const url = "http://api.icmdb.com/jokes/random";
const url1 = "http://api.icndb.com/jokes/random";
document.querySelector("button").addEventListener("click", () => {
    fetch(url1)
        .then(response => response.json())
        .then(data => {
            document.querySelector("main p").innerHTML = data.value.joke;
        }).catch(reject => console.error(reject.message))
});

