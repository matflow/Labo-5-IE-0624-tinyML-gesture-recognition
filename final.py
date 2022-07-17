

# In[25]:


# !echo "const unsigned char model[] = {" > model.h
# !cat gesture_model.tflite | xxd -i      >> model.h
# !echo "};"                              >> model.h


# In[ ]:
import os 
model_h_size = os.path.getsize("model.h")
print(f"Header ile, model.h is {model_h_size:,} bytes. ")
print("\nOpen the side panel (refresh if needed). Double click model.h to download file.")

