// Sticky menu

function MenuFixe() {
	var navbar = document.getElementById("menu");
	var sticky = 321;
  	if (window.pageYOffset >= sticky) {
    	navbar.classList.add("sticky");
  	} else {
    	navbar.classList.remove("sticky");
  	}
}