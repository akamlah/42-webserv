const options = {
	method: 'GET',
	headers: {
		accept: 'application/json',
		'X-RapidAPI-Key': 'SIGN-UP-FOR-KEY',
		'X-RapidAPI-Host': 'matchilling-chuck-norris-jokes-v1.p.rapidapi.com'
	}
};

fetch('https://matchilling-chuck-norris-jokes-v1.p.rapidapi.com/jokes/random', options)
	.then(response => response.json())
	.then(response => console.log(response))
	.catch(err => console.error(err));


// const url = "http://api.icmdb.com/jokes/random";
// const url1 = "http://api.icndb.com/jokes/random";
// document.querySelector("button").addEventListener("click", () => {
//     fetch(url1)
//         .then(response => response.json())
//         .then(data => {
//             document.querySelector("main p").innerHTML = data.value.joke;
//         }).catch(reject => console.error(reject.message))
// });

