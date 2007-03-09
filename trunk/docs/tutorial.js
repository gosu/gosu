function showLang(language)
{
    var elems = document.getElementsByTagName("div");
    for (var cur = 0; cur < elems.length; ++cur)
    {
        if (elems[cur].className == language || elems[cur].className == "")
            elems[cur].style.display = "block";
        else
            elems[cur].style.display = "none";
    }
}

function showLesson(lesson)
{
    var elems = document.getElementsByTagName("span");
    for (var cur = 0; cur < elems.length; ++cur)
    {
        if (elems[cur].className.substr(0, 6) == "lesson")
        {
            var firstLesson = parseInt(elems[cur].className.substr(6, 1));
            var lastLesson = parseInt(elems[cur].className.substr(8, 1));
            if (lesson >= firstLesson && lesson <= lastLesson)
                elems[cur].style.display = "inline";
            else
                elems[cur].style.display = "none";
            if (lesson == firstLesson)
                elems[cur].style.color = "red";
            else
                elems[cur].style.color = "black";
        }
    }
}