from PIL import Image
#http://www.celestiamotherlode.net/catalog/earth.html JestrEarth JPG 5 levels
def tx_name(x,y):
    return "tx_" +str(x) +"_"+str(y)+".jpg"

def merge_images(file1, file2):
    """Merge two images into one, displayed side by side
    :param file1: path to first image file
    :param file2: path to second image file
    :return: the merged Image object
    """
    image1 = Image.open(file1)
    image2 = Image.open(file2)

    (width1, height1) = image1.size
    (width2, height2) = image2.size

    result_width = width1 + width2
    #result_height = max(height1, height2)
    result_height = height1+ height2

    result = Image.new('RGB', (result_width, result_height))
    result.paste(im=image1, box=(0, 0))
    result.paste(im=image2, box=(width1, height1))
    return result
tiles_x = 32
tiles_y = 16
tile_size = (1024,1024)
virtual_texture_size = (tile_size[0] * tiles_x,tile_size[1] * tiles_y)
images_path ="JestrEarth JPG/textures/hires/JestrMarble JPG/level4/"
virtual_image = Image.new('RGB', (virtual_texture_size[0], virtual_texture_size[1]))

for x in range(tiles_x):
    for y in range(tiles_y-1):
        ptr_paste = [tile_size[0]*x,tile_size[1]*y]
        file_path = images_path + tx_name(x,y)
        file_path1 = images_path + tx_name(x,y+1)
        image1 = Image.open(file_path)
        image2 = Image.open(file_path1)
        virtual_image.paste(im=image1, box=(ptr_paste[0],ptr_paste[1]))
        print("x"+str(x))
        print("y"+str(y))
        ptr_paste[1] +=tile_size[0]
        virtual_image.paste(im=image2, box=(ptr_paste[0],ptr_paste[1]))
virtual_image.save("virtualTex4.tiff", format='TIFF', bigtiff=True)