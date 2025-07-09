import numpy as np
from tensorflow.keras.models import load_model
from datetime import datetime
import os
import librosa
import wave

# Load models
model = load_model("model3.h5")
model_emotions7 = load_model("model4.h5")
model_gender = load_model("model_mw.h5")

CAT6 = ["害怕", "愤怒", "中性", "高兴", "伤心", "惊讶"]
CAT7 = ["害怕", "厌恶", "中性", "高兴", "伤心", "惊讶", "愤怒"]
CAT3 = ["positive", "neutral", "negative"]

COLOR_DICT = {
    "neutral": "grey",
    "positive": "green",
    "happy": "green",
    "surprise": "orange",
    "fear": "purple",
    "negative": "red",
    "angry": "red",
    "sad": "lightblue",
    "disgust": "brown",
}

def get_mfccs(audio, limit):
    y, sr = librosa.load(audio)
    a = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=40)
    if a.shape[1] > limit:
        mfccs = a[:, :limit]
    elif a.shape[1] < limit:
        mfccs = np.zeros((a.shape[0], limit))
        mfccs[:, : a.shape[1]] = a
    return mfccs


def get_mfccs_new(audio, limit):
    with wave.open(audio, 'r') as wf:
        data = wf.readframes(-1)
        framerate = wf.getframerate()
    y = np.frombuffer(data, dtype=np.int16)
    a = librosa.feature.mfcc(y, sr=framerate, n_mfcc=40)
    if a.shape[1] > limit:
        mfccs = a[:, :limit]
    elif a.shape[1] < limit:
        mfccs = np.zeros((a.shape[0], limit))
        mfccs[:, : a.shape[1]] = a
    return mfccs

def get_title(predictions, categories=CAT6):
    title = f"Predicting emotions: {categories[predictions.argmax()]} - {predictions.max() * 100:.2f}%"
    return title

def color_dict(coldict=COLOR_DICT):
    return coldict

def predict_emotion_mfccs(audio_path, model, categories):
    mfccs = get_mfccs(audio_path, model.input_shape[-1])
    mfccs = mfccs.reshape(1, *mfccs.shape)
    pred = model.predict(mfccs)[0]
    return pred

def predict_gender(audio_path, model):
    gmfccs = get_mfccs(audio_path, model.input_shape[-1])
    gmfccs = gmfccs.reshape(1, *gmfccs.shape)
    gpred = model.predict(gmfccs)[0]
    return gpred

def predict_emotions7(audio_path, model, categories):
    mfccs_ = get_mfccs(audio_path, model.input_shape[-2])
    mfccs_ = mfccs_.T.reshape(1, *mfccs_.T.shape)
    pred_ = model.predict(mfccs_)[0]
    return pred_

def main(audio_path):
    # Example usage

    # Predict emotion using model with 6 categories
    # emotion_pred = predict_emotion_mfccs(audio_path, model, CAT6)
    # predicted_emotion = CAT6[np.argmax(emotion_pred)]
    # print(emotion_pred)
    # print("情感预测 (6 种):", predicted_emotion)

    # Predict emotion using model with 7 categories
    emotion_pred7 = predict_emotions7(audio_path, model_emotions7, CAT7)
    emotion_pred7[1] = emotion_pred7[1] / 1.8
    print(emotion_pred7)
    predicted_emotion7 = CAT7[np.argmax(emotion_pred7)]
    print("情感预测 (7 种):", predicted_emotion7)

    # Predict gender
    gender_pred = predict_gender(audio_path, model_gender)
    predicted_gender = "女性" if gender_pred[0] >= 0.5 else "男性"
    print("预测性别:", predicted_gender)

if __name__ == "__main__":
    main("chunks/6.wav")
