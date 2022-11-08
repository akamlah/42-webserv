                let x = '';
                let y = '';
                let curIndex = '';
                let images = document.querySelectorAll(".squer > img");
                let size = images.length;
                var arr = Array.prototype.slice.call(images);
                var modalImg = document.getElementById("img01");
                


                document.addEventListener("keydown", function(event){
                  var x = event.key;
                  if (x == "ArrowRight")
                  {
                    next(1);
                  }
                  else if ( x == "ArrowLeft") {
                    next(-1);
                  }
                  else {
                    return;
                  }
                } );
                function next(n) {
                  let littleSquer = document.getElementsByClassName("squer");
                  let noNe = 0;
                  size = images.length;
                         for (let m = 0; m < littleSquer.length; m++) {
                           
                           if (littleSquer[m].style.display == "none") {noNe += 1; }
                         }
                              if (littleSquer[curIndex].style.display == "none") {curIndex = 0;}
                          size = size - noNe; 
                         
                    if (curIndex + n > size-1 ) {
                        curIndex =-1;
                        let pictureselected = images[curIndex += n];
                        modalImg.src = newPath(pictureselected);
                    }
                    else if (curIndex + n < 0)
                    {
                        curIndex = size;
                        let pictureselected = images[curIndex += n];
                        modalImg.src = newPath(pictureselected);
                    }
                    else
                    {
                    let pictureselected = images[curIndex += n];
                    modalImg.src = newPath(pictureselected);
                    }
                    size= "";
                    littleSquer[curIndex].focus();

                }

                function newPath(cut) {
                    let pictureName = cut.src.split('/');
                    let z = pictureName.length;
                    let newPath = "./img/big/" + pictureName[z-2] + "/" + pictureName[z-1];
                   return newPath;  }
                document.addEventListener("click", function main() {
                    x = event.clientX;
                    y = event.clientY;
                    let current = document.elementFromPoint(x,y);
                    let orient = current.parentNode.className;
                    if (orient != "squer") {
                        return;
                    }
                    curIndex = arr.indexOf(current);
                    let path = newPath(current);
              /*   const test = document.elementFromPoint(x,y);
                let orient = test.className; */
               /*  let pictureName = current.src.split('/');
                let newPath = "http://127.0.0.1:5500/tecita/img/big/" + pictureName[6] + "/" + pictureName[7]; */
                /* newPath();
                let newPath = newPath(current); */
                // Get the modal
                var modal = document.getElementById('myModal');
                var textmod = document.getElementById('textmod');
                //console.log(newPath);
                // Get the image and insert it inside the modal - use its "alt" text as a caption
                textmod.style.display = "none";
                modal.style.display = "flex";
                document.getElementById("piccontrol").style.display = "flex";
                modalImg.src = path;
                //    if (orient == "allo") {      
                //     document.getElementsByClassName("modal-content")[0].style ="width:50%";
                // }
                // Get the <span> element that closes the modal
                var span = document.getElementsByClassName("close")[0];
                // When the user clicks on <span> (x), close the modal
                span.onclick = function() { 
                modal.style.display = "none";
                document.getElementById("piccontrol").style.display = "none";
                textmod.style.display = "flex";
                }
            });
           /* modalImg.style.width = '';*/
/*             window.onclick = function(event) {
              if (!event.target.matches('.mobildesign')) {
                console.log("fuck");
                let menu = document.getElementById('pressmenu');
                menu.style.display = "none";
              };
            };   */
 /*  function menudown() {
  let menu = document.getElementById('pressmenu');
  menu.style.display = "block"; */
  function openNav() {
    document.getElementById("mySidenav").style.width = "250px";
  }
  function closeNav() {
    document.getElementById("mySidenav").style.width = "0";
  }
  function closeLentNav(n) {
    if (n==1) {
    document.getElementById("LentSidenav").style.maxHeight = "0vh";
    document.getElementById("Large").style.height = "96vh";
    document.getElementById("lentClose").childNodes[1].src = "./img/uparrow2_89326.png";
    document.getElementById("lentClosetwo").childNodes[1].src = "./img/uparrow2_89326.png";

    /*document.getElementById("lentClose").setAttribute("onclick", "openLentNav");*/
    document.getElementById("lentClose").setAttribute("onclick", "closeLentNav(2)");
    document.getElementById("lentClosetwo").setAttribute("onclick", "closeLentNav(2)");
    }
    else if (n==2){
      /*document.getElementById("LentSidenav").style.height = "unset";*/
      document.getElementById("LentSidenav").style.maxHeight = "15vh";
      document.getElementById("Large").style.height = "81vh";
      document.getElementById("lentClose").childNodes[1].src = "./img/downarrow2_89392.png";
      document.getElementById("lentClosetwo").childNodes[1].src = "./img/downarrow2_89392.png";
      /*document.getElementById("lentClose").setAttribute("onclick", "closeLentNav");*/
      document.getElementById("lentClose").setAttribute("onclick", "closeLentNav(1)");
      document.getElementById("lentClosetwo").setAttribute("onclick", "closeLentNav(1)");
    }
  }
/*
  document.getElementById("lentClose").addEventListener("click", function openLentNav() {
    document.getElementById("LentSidenav").style.maxheight = "20%";
    document.getElementById("Large").style.maxHeight = "80%";
    document.getElementById("lentClose").childNodes[1].src = "http://127.0.0.1:5500/tecita/img/downarrow2_89392.png";
    document.getElementById("lentClose").setAttribute("onclick", "closeLentNav");
    console.log("fuck youuuu  2");
  });
*/
/* 
 function openLentNav() {
  document.getElementById("LentSidenav").style.maxheight = "20%";
  document.getElementById("Large").style.maxHeight = "80%";
  document.getElementById("lentClose").childNodes[1].src = "http://127.0.0.1:5500/tecita/img/downarrow2_89392.png";
  document.getElementById("lentClose").setAttribute("onclick", "closeLentNav");
};
 */

// Register touchstart and touchend listeners for element 'source'

var clientX, clientY;

modalImg.addEventListener('touchstart', function(e) {
  // Cache the client X/Y coordinates
  clientX = e.touches[0].clientX;
  clientY = e.touches[0].clientY;
  
}, false);



modalImg.addEventListener('touchend', function(e) {
  var deltaX, deltaY;

  // Compute the change in X and Y coordinates. 
  // The first touch point in the changedTouches
  // list is the touch point that was just removed from the surface.
  deltaX = e.changedTouches[0].clientX - clientX;
  deltaY = e.changedTouches[0].clientY - clientY;
  
  if (deltaX<deltaY) {
    next(1);
  }
  else if (deltaY<deltaX) {
    next(-1);
  }
  

  // Process the data ... 
}, false);

document.addEventListener( 'mouseup' , function(e) {
  if ( event.target != this.getElementById(mySidenav)) {
    closeNav();
  }
}

);


     