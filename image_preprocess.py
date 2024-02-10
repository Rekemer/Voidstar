from PIL import Image
import imageio
import cv2 
import os
Image.MAX_IMAGE_PIXELS = None
#http://www.celestiamotherlode.net/catalog/earth.html JestrEarth JPG 5 levels
def tx_name(x,y):
    return "tx_" +str(x) +"_"+str(y)+".jpg"

def GenerateMipMaps(path,dirname):
    img = cv2.imread(path)
    height,width,_ = img.shape
    if not os.path.isdir(dirname):
        os.mkdir(dirname)
    while width != 0 and height != 0:
        print("x"+str(width))
        print("y"+str(height))
        res = cv2.resize(img, dsize=(int(width), int(height)), interpolation=cv2.INTER_LINEAR)
        cv2.imwrite( os.path.join(dirname, str(width) +"_"+str(height)+".tiff"), res) 
        width//=2
        height//=2

def GeneratePages(page_size_x,page_size_y,mip_maps,max_x,max_y):
    width, height = max_x,max_y
    while width >= page_size_x and height >= page_size_y:
        mipMap = Image.open(mip_maps +"/" +  str(width) +"_"+str(height) +".tiff")
        page_amount_x = width // page_size_x
        page_amount_y = height // page_size_y
        
        pages_dir=os.path.join(mipMaps , "pages_" +  str(width) +"_"+str(height))
       # print (pages_dir)
       # return
        if not os.path.isdir(pages_dir):
            os.mkdir(pages_dir)
        for x in range(page_amount_x):
            for y in range(page_amount_y):
                left =page_size_x*x
                right=page_size_x*x+page_size_x
                bottom=page_size_y*y+page_size_y
                top=page_size_y*y
                subimage = mipMap.crop((left, top, right, bottom))
                subimage.save(pages_dir + "/"+str(x) +"_"+ str(y) +".png")

        width//=2
        height//=2

tiles_x = 64
tiles_y = 32
page_size_x  = 128
page_size_y  = 64

tile_size = (1024,1024)
virtual_texture_size = (tile_size[0] * tiles_x,tile_size[1] * tiles_y)
images_path ="JestrEarth JPG/textures/hires/JestrMarble JPG/level5/"
virtual_image = Image.new('RGB', (tile_size[0]*2, tile_size[1]*2))

tile = (2,4)
file_path00 = images_path + tx_name(tile[0],tile[1])
file_path01 = images_path + tx_name(tile[0],tile[1]+1)
file_path11 = images_path + tx_name(tile[0]+1,tile[1]+1)
file_path10 = images_path + tx_name(tile[0]+1,tile[1])
image00 = Image.open(file_path00)
image01 = Image.open(file_path01)
image10 = Image.open(file_path10)
image11 = Image.open(file_path11)
ptr_paste = [0,0]
virtual_image.paste(im=image00, box=(ptr_paste[0],ptr_paste[1]))
ptr_paste[0] +=tile_size[0]
virtual_image.paste(im=image10, box=(ptr_paste[0],ptr_paste[1]))
ptr_paste[1] +=tile_size[1]
virtual_image.paste(im=image11, box=(ptr_paste[0],ptr_paste[1]))
ptr_paste[0] -=tile_size[0]
virtual_image.paste(im=image01, box=(ptr_paste[0],ptr_paste[1]))
#virtual_image.show()
pagesPerTile = (int(tile_size[0]/page_size_x),int(tile_size[1]/page_size_y))
print(pagesPerTile)
coords = [0,0]
MAX_TILES_5_LEVEL = [1024* 64 / 128,1024* 32 / 64];
tlieCoords = [0,0]  
for x in range(tiles_x):
    for y in range(tiles_y):
        tlieCoords[0] = x*pagesPerTile[0]
        file_path = images_path + tx_name(x,y)
        image = Image.open(file_path)
        print("Loading: " + file_path)
        for xx in range(pagesPerTile[0]):
            tlieCoords[1] = y*pagesPerTile[1]
            for yy in range(pagesPerTile[1]):
                coords = [xx*page_size_x,yy*  page_size_y] 
                left = coords[0] 
                top = coords[1] 
                right =  coords[0] + page_size_x
                bottom =  coords[1] + page_size_y
                subimage = image.crop((left, top, right, bottom))
                pages_dir=os.path.join("virt5" , "pages_")
                if not os.path.isdir(pages_dir):
                    os.makedirs(pages_dir)
                
                savePath  = pages_dir + "/"+str(tlieCoords[0]) +"_"+ str(tlieCoords[1]) +".png"
                tlieCoords[1]+=1
                #print(savePath)
                subimage.save(savePath)
            tlieCoords[0]+=1
        
# image00.show()
# image01.show()
# image10.show()
# image11.show()
# for x in range(tiles_x):
#     for y in range(tiles_y-1):
#         ptr_paste = [tile_size[0]*x,tile_size[1]*y]
#         file_path = images_path + tx_name(x,y)
#         file_path1 = images_path + tx_name(x+1,y+1)
#         file_path2 = images_path + tx_name(x+1,y)
#         image1 = Image.open(file_path)
#         image2 = Image.open(file_path1)
#         image3 = Image.open(file_path2)

#         image01.paste(im=image1, box=(ptr_paste[0],ptr_paste[1]))
#         virtual_image.paste(im=image1, box=(ptr_paste[0],ptr_paste[1]))
#         virtual_image.paste(im=image1, box=(ptr_paste[0],ptr_paste[1]))
#         #print("x"+str(x))
#         #print("y"+str(y))
#         ptr_paste[1] +=tile_size[0]
#         virtual_image.paste(im=image2, box=(ptr_paste[0],ptr_paste[1]))
#imageio.imwrite("virtualTex5.dds", virtual_image)
#virtual_image.save("virtualTex4.tiff", format='TIFF', bigtiff=True)




path = "virtualTex4.tiff"
virtual_texture= Image.open(path)

width, height = virtual_texture.size

mipMaps = "mipMaps_"+str(path)

#GeneratePages(page_size_x,page_size_y,mipMaps,width,height)
#GenerateMipMaps(path,mipMaps)
