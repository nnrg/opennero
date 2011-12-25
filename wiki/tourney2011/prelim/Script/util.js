function toggleVisibility(t)
{
  var myDisplay = '';
  if (document.getElementById)
  {
    myDisplay = (document.getElementById(t).style.display == 'none') ? 'block' : 'none';
    document.getElementById(t).style.display = myDisplay;
  } else {
    alert('oops');
  }
}
