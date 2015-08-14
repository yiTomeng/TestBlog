package com.yly;

import java.io.File;
import java.io.IOException;
import java.util.Hashtable;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.WriterException;
import com.google.zxing.common.ByteMatrix;
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;

public class QRCodeCreator {

    public static void main(String[] args) {
        // TODO 自動生成されたメソッド・スタブ
        String text = "http://www.google.co.jp";
        int width = 300;
        int height = 300;
        String format = "jpeg";

        Hashtable<EncodeHintType, Object> hints = new Hashtable<EncodeHintType, Object>();
        hints.put(EncodeHintType.CHARACTER_SET, "utf-8");
        hints.put(EncodeHintType.ERROR_CORRECTION, ErrorCorrectionLevel.M);
        
        ByteMatrix bitMatrix;
        try {
            bitMatrix = new MultiFormatWriter().encode(text, BarcodeFormat.QR_CODE, width, height, hints);

            File outputFile = new File("C:\\img" + File.separator + "new.jpeg");

            MatrixToImageWriter.writeToFile(bitMatrix, format, outputFile);
        } catch (IOException | WriterException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        }
    }

}
