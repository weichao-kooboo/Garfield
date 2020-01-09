from django.shortcuts import render
import json
from django.shortcuts import HttpResponse
# Create your views here.

user_list = []

def index(request):
    # return HttpResponse("hello world!")
    if request.method == "POST":
        username = request.POST.get("username", None)
        password = request.POST.get("password", None)
        # print(username+password)
        print(request.headers)
        temp = {"user": username, "pwd": password}
        user_list.append(temp)
        concat = request.POST
        postbody=request.body
        print(concat)
        print(type(postbody))
        print(postbody)
        my_json = str(postbody)
        print(my_json)
    return render(request,"index.html",{"data":user_list})