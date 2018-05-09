#!/usr/bin/python
from gimpfu import register, PF_IMAGE, PF_DRAWABLE, PF_STRING, PF_OPTION, main,gimp

gamma_corr = \
[0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,\
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,\
1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,\
2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,\
5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,\
10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,\
17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,\
25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,\
37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,\
51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,\
69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,\
90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,\
115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,\
144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,\
177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,\
215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255]

def array2struct(data): 
    return "{%(i)d,%(j)d,%(k)d}" % {"i":ord(data[0]), "j":ord(data[1]), "k":ord(data[2])}

def packColor(data):
    r5 = (gamma_corr[ord(data[0])] * 249 + 1014) >> 11
    g6 = (gamma_corr[ord(data[1])] * 253 + 505) >> 10
    b5 = (gamma_corr[ord(data[2])] * 249 + 1014) >> 11
    #return ",".join([str(r5), str(g6), str(b5)])
    return "{0:#06x}".format((r5 << 11) & 0xf800 | (g6 << 5) & 0x07c0 | (b5 & 0x003f))
    
def exportCcode(timg, tdrawable,iconname,filename,colormode):
    pr =tdrawable.get_pixel_rgn(0, 0, timg.width,timg.height, False, False);
    i=0
    j=0
    size = timg.width*timg.height #LCD size
    output = "/*\nC code Exported from Gimp\n\nicon size in pixels: width="+str(timg.width)+", height:"+str(timg.height)+"\n*/\n\n" 
    if colormode==0:
        gimp.message("12-bit mode not yet implemented")
    elif colormode==1:
        output += "uint16_t "+iconname+"[" + "%(size)d" % {"size":size} + "]={\n" 
        while (j<timg.height-1):
            while (i<timg.width-1):
                output += packColor(pr[i,j]) + ","
                i+=1
            output += packColor(pr[i,j]) #the last column, no comma
            output += ",\n" #end of each row
            j+=1
            i=0
        while (i<timg.width-1): #the last row, no comma
            output += packColor(pr[i,j]) + ","
            i+=1
        output += packColor(pr[i,j]) #the last column, no comma
        output += "\n};"
        savefile = open(filename,"w")
        savefile.write(output)
        savefile.close() 
    elif colormode==2:
        gimp.message("18-bit mode not yet implemented")
    elif colormode==3:
        output += "typedef struct{ \nunsigned char \nr,\ng,\nb;\n}RGB;\n\nRGB "+iconname+"[" + "%(size)d" % {"size":size} + "]={\n" 
        while (j<timg.height-1):
            while (i<timg.width-1):
                output += array2struct(pr[i,j]) + ","
                i+=1
            output += array2struct(pr[i,j]) #the last column, no comma
            output += ",\n" #end of each row
            j+=1
            i=0
        while (i<timg.height-1): #the last row, no comma
            output += array2struct(pr[i,j]) + ","
            i+=1
        output += array2struct(pr[i,j]) #the last column, no comma
        output += "\n};"
        savefile = open(filename,"w")
        savefile.write(output)
        savefile.close() 
    else:
        gimp.message("Error: No color mode selected")#will hardly happen as we now have a default colormode   
  
    
register(
        "export-C-code",
        "Saves the image data for LCD C programming",
        "Saveimage data for LCD C programming",
        "David Muriuki Karibe",
        "David Muriuki Karibe",
        "2015",
        "export C code",
        "RGB*, GRAY*",
        [
         (PF_IMAGE, "timg", "Input image", None),
         (PF_DRAWABLE, "tdrawable", "Input drawable", None),
         (PF_STRING, "iconname", "Icon variable name","icon"),
         (PF_STRING,"filename","file to save code to","untitled.c"),
         (PF_OPTION,"colormode","RGB Color Mode",1,("12-bit(4-4-4): not implemented","16-bit(5-6-5)", "18-bit(6-6-6): not implemented","24-bit(8-8-8)"))
        ],
        [],#no return params from the plugin function
        exportCcode, menu="<Image>/File/Export/")

main()