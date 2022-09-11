/* const promise = new Promise(function (resolve, reject) {

    resolve("sikeres");
    reject("Buuuu is not good");
});

console.log(promise);

const asypromise = new Promise(function (resolve, reject) {
    setTimeout(() => resolve("sikeres"), 100);
    reject("Buuuu is not good");
});
console.log(asypromise);
setTimeout(() => console.log(asypromise), 100);

asypromise.then(
    function ongood(data) { },
    function ongbad(reason) {

    },
);
 */
const one = {
    thequestion() {
        return new Promise(function (know, notknow) {
            setTimeout(know, 100, "yes", "second", "third");
            //notknow("I dont know the question");
        });
    }
};

const two = {
    theanswer() {
        one.thequestion().
            then(
                function yes(data) {
                    console.log(arguments);
                    console.log("The anwer is yes", data);
                },
                function no(reason) {
                    console.log("The anwer is no", reason);
                }
            );
    }
};
//two.theanswer();
const pizza = new Promise((resolve, reject) => {
    //setTimeout(resolve, 100, ["Pizza quatro formaggio", "cola"]);
    reject('was a problem');
});

const deliver = pizza.then(food => {
    console.log("here is your order", food);
    if (!food.includes("teve")) {
        return new Promise((resolve, reject) => { setTimeout(resolve, 100, ["kávé", "süti"]); });
    }

})
    .then(newFood => {
        console.log("second delivery", newFood);
    });

console.log(pizza, deliver);