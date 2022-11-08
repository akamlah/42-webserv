let toChange = document.getElementById("LentSidenav");
let toChangeSquer = document.getElementsByClassName("squer");
let runner = toChange.querySelectorAll("img");
let n = "";
let bigPicContainer = {
    0: ["SP_A0751.jpg","SP_A0753.jpg","SP_A0755.jpg","SP_A0756.jpg","SP_A0757.jpg","SP_A0758_allo.jpg","SP_A0759_allo.jpg","SP_A0760.jpg","SP_A0761_allo.jpg","SP_A0762_allo.jpg","SP_A0763_allo.jpg","SP_A0765.jpg","SP_A0767.jpg","SP_A0768.jpg","SP_A0769.jpg","SP_A0773_allo.jpg","SP_A0774.jpg","SP_A0776.jpg","SP_A0777.jpg","SP_A0778_allo.jpg","SP_A0779.jpg","SP_A0781_allo.jpg","SP_A0782_allo.jpg","SP_A0783.jpg","SP_A0787_allo.jpg"],
    1: ["357-DSCF4153.jpg","358-DSCF4154.jpg","359-DSCF4155_allo.jpg","360-DSCF4156.jpg","361-DSCF4157.jpg","362-DSCF4158.jpg","363-DSCF4161.jpg","365-DSCF4163.jpg","366-DSCF4164.jpg","367-DSCF4165.jpg"],
    2: ["044-DSCF5080_mirrorMe_allo.jpg","045-DSCF5089_mirrorMe.jpg","046-DSCF5096_mirrorMe.jpg","047-DSCF5097_mirrorMe.jpg","048-DSCF5101_mirrorMe_allo.jpg","120-DSCF5620_mirrorMe_allo.jpg","326-DSCF8572_mirrorMe.jpg","329-DSCF1504.jpg","341-DSCF1864_mirrorMe_allo.jpg","351-DSCF2830_mirrorMe_allo.jpg","353-DSCF2834_mirrorMe_allo.jpg","370-DSCF0218_mirrorMe.jpg","390-DSCF9229_mirrorMe.jpg","396-DSCF6418_mirrorMe_allo.jpg","480-DSCF7192_mirrorMe.jpg","532-DSCF2805_mirrorMe.jpg","609-DSC_1143_mirrorMe.jpg","613-DSC_0588_mirrorMe.jpg","615-DSC_0060.jpg","616-DSC_0061_allo.jpg","617-DSC_0063_mirrorMe_allo.jpg","618-DSC_0071_mirrorMe.jpg","621-DSC_0095_mirrorMe.jpg","639-DSC_0465_mirrorMe.jpg","641-DSC_0035 (4).jpg","652-DSC_1169_mirrorMe.jpg","662-DSC_0489_mirrorMe.jpg","667-DSC_0270.jpg","686-DSC_0132_mirrorMe_allo.jpg","693-DSC_0206_mirrorMe_allo.jpg","709-DSC_0332 (3)_mirrorMe.jpg","733-DSC_0602_mirrorMe_allo.jpg","736-DSC_0689_mirrorMe.jpg","741-DSC_0108_mirrorMe.jpg","742-DSC_0115_mirrorMe.jpg","744-DSC_0126_mirrorMe.jpg","758-DSC_0256_mirrorMe.jpg","779-DSC_0447 (3)_mirrorMe_allo.jpg","799-DSC_0185 (3)_mirrorMe.jpg","800-DSC_0189 (3)_mirrorMe.jpg","809-DSC_0250_mirrorMe_allo.jpg","810-DSC_0257_mirrorMe_allo.jpg"],
    3: ["032-DSCF4992_Big_Life_allo.jpg","036-DSCF5011_Big_Life_allo.jpg","129-DSCF3484_Big_Life_allo.jpg","138-DSCF3612_Big_Life.jpg","140-DSCF3653_Big_Life_allo.jpg","159-DSCF4440.jpg","164-DSCF4553_Big_Life_allo.jpg","176-DSCF1465_Big_Life.jpg","179-DSCF1498_Big_Life_allo.jpg","180-DSCF1503_allo.jpg","209-DSCF9672_Big_Life_allo.jpg","273-DSCF1212_squer_Life.jpg","275-DSCF1233.jpg","280-DSCF1281_squer_Life.jpg","296-DSCF5949_squer_Life.jpg","313-DSCF6311_Big_Life.jpg","394-DSCF6318_Big_Life.jpg","398-DSC_0269_Big_Life_allo.jpg","399-DSC_0351_Big_Life_allo.jpg","400-DSC_0459_Big_Life_allo.jpg","401-DSC_1084_Big_Life_allo.jpg","478-DSCF7047_Big_Life_allo.jpg","494-DSCF5158_Big_Life.jpg","543-DSCF6822_Big_Life.jpg","559-DSCF8556_Big_Life.jpg","635-DSC_0443_Big_Life_allo.jpg","642-DSC_0039 (4)_Big_Life_allo.jpg","646-DSC_0116_Big_Life.jpg","650-DSC_0592_Big_Life_allo.jpg","664-DSC_1066.jpg","687-DSC_0166_Big_Life.jpg","699-DSC_0244 (3)_allo.jpg","700-DSC_0247 (3)_Big_Life_allo.jpg","705-DSC_0291 (3)_Big_Life_allo.jpg","706-DSC_0301 (3)_Big_Life_allo.jpg","707-DSC_0313 (3)_Big_Life_allo.jpg","711-DSC_0346 (3)_Big_Life_allo.jpg","712-DSC_0362_Big_Life_allo.jpg","717-DSC_0410_Big_Life_allo.jpg","722-DSC_0434_Big_Life_allo.jpg","770-DSC_0296.jpg","777-DSC_0385 (3)_Big_Life_allo.jpg","788-DSC_0502 (3)_Big_Life_allo.jpg"],
    4: ["334-DSCF1627_allo.jpg","355-DSCF2974.jpg","356-DSCF3246_allo.jpg","496-DSCF0018_squer_Life.jpg","498-DSCF0304_squer_Life.jpg","560-DSCF8602.jpg","565-DSCF8718.jpg","566-DSCF8768.jpg","567-DSCF8782.jpg","568-DSCF8800.jpg","570-DSCF8821.jpg","571-DSCF8834.jpg","572-DSCF8840.jpg","573-DSCF8841.jpg","575-DSCF8858.jpg","578-DSCF8867_allo.jpg","579-DSCF8873_allo.jpg","580-DSCF8878.jpg","654-DSC_0164_object.jpg"],
    5: ["062-DSCF5910.jpg","069-DSCF5973_hand.jpg","070-DSCF5979_hand_allo.jpg","071-DSCF5981_hand_allo.jpg","072-DSCF5983_hand_allo.jpg","075-DSCF6040_hand.jpg","077-DSCF6148_hand.jpg","078-DSCF6192_hand.jpg","080-DSCF8143_hand.jpg","082-DSCF8273_hand.jpg","116-DSCF9269.jpg","117-DSCF5543_hand.jpg","236-DSCF0977.jpg","283-DSCF1341.jpg","311-DSCF6070_squer_Life.jpg","470-DSCF7001_hand.jpg","473-DSCF7010_hand.jpg","484-DSCF7276.jpg","485-DSCF7280_hand.jpg","516-DSCF9927_hand.jpg","517-DSCF9946.jpg","591-DSCF5771_hand.jpg","595-DSCF5798_hand.jpg","597-DSCF5800_hand_allo.jpg","599-DSCF5806_hand.jpg","600-DSCF5812.jpg","660-DSC_0016 (3)_hand.jpg","680-DSC_0355_hand.jpg"],
    6: ["016-DSCF1369_object.jpg","088-DSCF8711_object.jpg","112-DSCF9236_object.jpg","242-DSCF0999_object.jpg","258-DSCF1084_view.jpg","286-DSCF4893_object.jpg","288-DSCF4944_view.jpg","328-DSCF8583_object.jpg","495-DSCF0012_object.jpg","497-DSCF0023_object.jpg","506-DSCF9849_object.jpg","511-DSCF9888_object.jpg","518-DSCF9958_object.jpg","519-DSCF9959_object.jpg"],
    7: ["085-DSCF8675_view.jpg","086-DSCF8703.jpg","111-DSCF9235.jpg","168-DSCF8497_view.jpg","270-DSCF1171_view.jpg","309-DSCF6065_view.jpg","315-DSCF6525_view.jpg","317-DSCF6571_view.jpg","318-DSCF6572_view.jpg","402-DSCF1436_view.jpg","499-DSCF9792_view.jpg","503-DSCF9839_view.jpg","505-DSCF9848_view.jpg","507-DSCF9857_view.jpg","508-DSCF9878_view.jpg","514-DSCF9906_view.jpg","521-DSCF9967_view.jpg","820-DSCF6627_view.jpg","821-DSCF6628.jpg","822-DSCF6631.jpg","repet 2-fin.jpg"],
    8: ["089-DSCF8752_make.jpg","090-DSCF8770_make.jpg","091-DSCF8779_make.jpg","092-DSCF8789_make.jpg","093-DSCF8801_make.jpg","094-DSCF8817_make.jpg","095-DSCF8825_make.jpg","096-DSCF8837_make.jpg","097-DSCF8846_make.jpg","101-DSCF8952_make.jpg","102-DSCF9025_make.jpg","103-DSCF9083_make.jpg","104-DSCF9097_make.jpg","108-DSCF9226_make.jpg","427-DSCF4778_make.jpg","433-DSCF4798_make.jpg","434-DSCF4799_make.jpg","435-DSCF4800_make.jpg","436-DSCF4803_make.jpg","437-DSCF4805_make.jpg","438-DSCF4814_make.jpg","439-DSCF4816_make.jpg","440-DSCF4823_make.jpg","441-DSCF4824_make.jpg","523-DSCF9979_make.jpg","524-DSCF9990_make.jpg","536-DSCF6715_make.jpg","537-DSCF6725_make.jpg","538-DSCF6728_make.jpg","539-DSCF6738_make.jpg","540-DSCF6748_make.jpg","541-DSCF6759_make.jpg","542-DSCF6764_make.jpg"],
    9: ["039-DSCF5021_allo.jpg","040-DSCF5029.jpg","154-DSCF3844_spy.jpg","156-DSCF3872_spy_allo.jpg","172-DSCF0195_spy_allo.jpg","177-DSCF1469_spy.jpg","197-DSCF7929_spy_allo.jpg","199-DSCF7960_spy_allo.jpg","201-DSCF7981_spy_allo.jpg","218-DSCF9738_spy_allo.jpg","346-DSCF2752_view_allo.jpg","348-DSCF2759_view.jpg","380-DSCF1327_spy.jpg","426-DSCF4769_spy_allo.jpg","449-DSCF8150_spy_allo.jpg","610-DSC_0531_spy.jpg","637-DSC_0457_spy_allo.jpg","643-DSC_0044 (4)_spy.jpg","651-DSC_0602_allo.jpg","731-DSC_0555_spy.jpg","735-DSC_0676_spy_allo.jpg","771-DSC_0298_spy.jpg","774-DSC_0332_spy_allo.jpg","775-DSC_0337_spy_allo.jpg","776-DSC_0362 (3)_spy_allo.jpg","785-DSC_0483 (3)_spy.jpg","798-DSC_0171 (3)_spy.jpg"],
    10:["053-DSCF5207_allo.jpg","056-DSCF5418_view.jpg","137-DSCF3608_view_allo.jpg","151-DSCF3826_view.jpg","162-DSCF4523_view_allo.jpg","375-DSCF1009_view.jpg","395-DSCF6343_view_allo.jpg","447-DSCF8139_view.jpg","455-DSCF8230_view_allo.jpg","457-DSCF6911_view_allo.jpg","459-DSCF6920_view.jpg","493-DSCF5156_view.jpg","525-DSCF2042_view.jpg","531-DSCF2368_view.jpg","545-DSCF6970_view.jpg","582-DSCF8896_allo.jpg","604-DSCF5862_view_allo.jpg","606-DSC_1067_view.jpg","619-DSC_0087_view.jpg","631-DSC_0159_view.jpg","633-DSC_0194_view.jpg","663-DSC_1046_view.jpg","665-DSC_1093_view_allo.jpg","698-DSC_0242_view_allo.jpg","710-DSC_0340 (3)_view_allo.jpg","724-DSC_0456_view_allo.jpg","754-DSC_0235_view_allo.jpg","772-DSC_0299_view_allo.jpg","778-DSC_0437 (3)_view.jpg","812-DSC_0071 (2)_view_allo.jpg","814-DSC_0087 (2)_view_allo.jpg"],
    11:["003-DSCF6604_allo.jpg","043-DSCF5069_object.jpg","049-DSCF5103_object.jpg","050-DSCF5119_object.jpg","054-DSCF5328_object_allo.jpg","084-DSCF8297_object.jpg","119-DSCF5611.jpg","169-DSCF0078_object.jpg","173-DSCF0302_object.jpg","221-DSCF9749_object.jpg","335-DSCF1670_object.jpg","352-DSCF2833_object.jpg","373-DSCF0913_object.jpg","392-DSCF9279_object.jpg","444-DSCF8102_object.jpg","445-DSCF8111_object.jpg","446-DSCF8134_object.jpg","452-DSCF8213_object.jpg","454-DSCF8226_object.jpg","456-DSCF6891_object.jpg","461-DSCF6933_object_allo.jpg","464-DSCF6955_object_allo.jpg","612-DSC_0583_object_allo.jpg","614-DSC_0037_object.jpg","620-DSC_0093_object_allo.jpg","629-DSC_0138_object_allo.jpg","653-DSC_0080_object.jpg","666-DSC_0266_object.jpg","684-DSC_0065 (3)_object.jpg","704-DSC_0282 (3)_object_allo.jpg","734-DSC_0625_object_allo.jpg"],
    12:["front.jpg","kollazsok4.jpg","kollazsok5.jpg","smoke face tre.jpg","smoke face tre1.jpg","smoke face tre2.jpg","zero 2-fin.jpg", "bn_square.jpg"]
};

let foldernames = [
    "See_what_I_see_00",
    "bluredinside_01",
    "just_hiding_mirror_02",
    "big_life_03",
    "forgotten_life_living_past_04",
    "rincles_of_time_living_past_05",
    "objecting_world_squer_object_06",
    "frozen_time_squer_view_07",
    "can_you_remeber_squer_08",
    "widows_of_us_09",
    "Standing_there_view_10",
    "Meaningfull findings_object_11",
    "catched_moments_12"
];

let textContainer = [
    "See_what_I_see_00",
    "bluredinside_01",
    "just_hiding_mirror_02",
    "big_life_03",
    "forgotten_life_living_past_04",
    "You can look ate my eyes and see who i am. You can look at my hands and see what i did. Time is there. No end or beggining. If you learn to read it its like a book with thousand of pages. REmeberes every day and every moment.",
    "objecting_world_squer_object_06",
    "frozen_time_squer_view_07",
    "can_you_remeber_squer_08",
    "widows_of_us_09",
    "Standing_there_view_10",
    "Meaningfull findings_object_11",
    "catched_moments_12"
];

let titleNamesContanier = {
    9 : "Windows of Us",
0 :"See what I see",
12 :"Catched in action",
10 :"Standing There",
3:"Big Life",
11:"Meaningful findings",
4:"Forgotten life",
5:"wrinkles of Time",
5555:"Viatent Life" ,  
7:"Frozen Time",
4444:"Dropped Down",
8:"Can you Remember?",
6:"Objecting World",
2222:"Dancing variations",
1:"Blurred Inside " ,
2:"Just hiding"
    
};

function openPicmap(n) { 
    document.getElementById('myModal').style.display="none";
    document.getElementById('indexNone').style.display="block";
    
    let text = document.getElementById("textmod");
    text.style.display = "flex";
    text.children[0].innerHTML = textContainer[n];
    document.getElementById("photoTitle").innerHTML = titleNamesContanier[n];
    
    
       if ( bigPicContainer[n].length <= runner.length ) {
                let z= bigPicContainer[n].length;   
                for (let k = 0; k < z; k++) {
                    runner[k].src = "./img/small/" + foldernames[n] + "/" + bigPicContainer[n][k];
                    let chekc = bigPicContainer[n][k].includes("allo");
                    if (chekc == true ) {runner[k].setAttribute("class", "allo");}
                    else {runner[k].setAttribute("class", "fekvo");}
                    toChangeSquer[k].style.display = "inline-block";                   
                }
                for (let m = z; m < runner.length; m++) {
                   /* runner[m].src = "./img/small/" + foldernames[n] + "/" + bigPicContainer[n][m-z];*/
                    toChangeSquer[m].style.display = "none";
                }
        }
        else {
           /* let z= toChange.length;  
            for (let k = 0; k < z; k++) {
                runner[k].src = "./img/small/" + foldernames[n] + "/" + bigPicContainer[n][k];
            }
                here is to create the div squer if its not existing already
            for (let m = z; m < bigPicContainer[n].length; m++) {
                const element = array m];
                
            }
*/  
        console.log("helloooooo");
        console.log(bigPicContainer[n].length);
        console.log(runner.length);
        }


}


