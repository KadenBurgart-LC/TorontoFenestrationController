
// REFRESH BUTTON VISUAL EFFECTS
var refreshHoverOn = function(){ $(this).attr("src", "./assets/Refresh2Hover.png"); };
var refreshHoverOff = function(){ $(this).attr("src", "./assets/Refresh2.png"); };

$('.refresh>img').hover(refreshHoverOn, refreshHoverOff);

var refreshButtonLoadingMode = function(btnImg){
  btnImg.unbind('mouseenter mouseleave');
  btnImg.attr('src', './assets/spinner.gif');
}

var refreshButtonReadyMode = function(btnImg){
  btnImg.attr('src', './assets/Refresh2.png');
  btnImg.hover(refreshHoverOn, refreshHoverOff);
};