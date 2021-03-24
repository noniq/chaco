import java.io.*;
import java.util.Arrays;

/*
    18/01/2020 Geert De Prins
    Convert the turbo Chameleon help file to PETSCII and rebuild the file from PETSCII
    Crude UTF8<->Petscii conversion added by Tobias Korbmacher
*/

public class ChamHelp {
    private final static int MAXPAGES = 9999;
    private static final String utf8BOM = "\uFEFF";

    private static long getOffset(byte[] buf) {
        long result = ((long)buf[1] & 0xFF) + (((long)buf[2] & 0XFF) << 8) + (((long)buf[3] & 0XFF) << 16);
        return result;
    }

    private static int getPageNum(byte[] buf) {
        int result = ((int)buf[1] & 0xFF) + (((int)buf[2] & 0XFF) << 8);
        return result;
    }

    private static int getLink(long[] posList, long offSet, int iPages) {
        int begin,end,mid;

        begin=0;end=iPages-1;
        do {
            mid = (begin + end) >> 1;
            long elem = posList[mid];
            if (elem < offSet) {
                begin = mid+1;
            } else if (elem > offSet)  {
                end = mid-1;
            } else {
                return mid;
            }
        } while (begin <= end);

        return 0;
    }

    private static int outputPetsciiByte(BufferedWriter bufferedWriter, int value) {
//        String colText = String.format("<C$%02X>",value & 0xff);
        String[] petStr = {
        "<c$00>", "<c$01>", "<c$02>", "<c$03>", "<c$04>", "<c$05>", "<c$06>", "<c$07>", "<c$08>", "<c$09>", "<c$0a>", "<c$0b>", "<c$0c>", "<c$0d>", "<c$0e>", "<c$0f>",
        "<c$10>", "<c$11>", "<c$12>", "<c$13>", "<c$14>", "<c$15>", "<c$16>", "<c$17>", "<c$18>", "<c$19>", "<c$1a>", "<c$1b>", "<c$1c>", "<c$1d>", "<c$1e>", "<c$1f>",
//        "<c$20>", "<c$21>", "<c$22>", "<c$23>", "<c$24>", "<c$25>", "<c$26>", "<c$27>", "<c$28>", "<c$29>", "<c$2a>", "<c$2b>", "<c$2c>", "<c$2d>", "<c$2e>", "<c$2f>",
             " ",      "!",     "\"",      "#",      "$",      "%",      "&",      "'",      "(",      ")",      "*",      "+",      ",",      "-",      ".",      "/",
//        "<c$30>", "<c$31>", "<c$32>", "<c$33>", "<c$34>", "<c$35>", "<c$36>", "<c$37>", "<c$38>", "<c$39>", "<c$3a>", "<c$3b>", "<c$3c>", "<c$3d>", "<c$3e>", "<c$3f>",
             "0",      "1",      "2",      "3",      "4",      "5",      "6",      "7",      "8",      "9",      ":",      ";",      "<",      "=",      ">",      "?",
//        "<c$40>", "<c$41>", "<c$42>", "<c$43>", "<c$44>", "<c$45>", "<c$46>", "<c$47>", "<c$48>", "<c$49>", "<c$4a>", "<c$4b>", "<c$4c>", "<c$4d>", "<c$4e>", "<c$4f>",
             "@",      "a",      "b",      "c",      "d",      "e",      "f",      "g",      "h",      "i",      "j",      "k",      "l",      "m",      "n",      "o",
//        "<c$50>", "<c$51>", "<c$52>", "<c$53>", "<c$54>", "<c$55>", "<c$56>", "<c$57>", "<c$58>", "<c$59>", "<c$5a>", "<c$5b>", "<c$5c>", "<c$5d>", "<c$5e>", "<c$5f>",
             "p",      "q",      "r",      "s",      "t",      "u",      "v",      "w",      "x",      "y",      "z",      "[", "<c$5C>",      "]",      "^",      "_",

        "<c$60>", "<c$61>", "<c$62>", "<c$63>", "<c$64>", "<c$65>", "<c$66>", "<c$67>", "<c$68>", "<c$69>", "<c$6a>", "<c$6b>", "<c$6c>", "<c$6d>", "<c$6e>", "<c$6f>",
        "<c$70>", "<c$71>", "<c$72>", "<c$73>", "<c$74>", "<c$75>", "<c$76>", "<c$77>", "<c$78>", "<c$79>", "<c$7a>", "<c$7b>", "<c$7c>", "<c$7d>", "<c$7e>", "<c$7f>",

//        "<c$60>",      "A",      "B",      "C",      "D",      "E",      "F",      "G",      "H",      "I",      "J",      "K",      "L",      "M",      "N",      "O",
//             "P",      "Q",      "R",      "S",      "T",      "U",      "V",      "W",      "X",      "Y",      "Z", "<c$7B>", "<c$7C>", "<c$7D>", "<c$7E>", "<c$7F>",

        "<c$80>", "<c$81>", "<c$82>", "<c$83>", "<c$84>", "<c$85>", "<c$86>", "<c$87>", "<c$88>", "<c$89>", "<c$8a>", "<c$8b>", "<c$8c>", "<c$8d>", "<c$8e>", "<c$8f>",
        "<c$90>", "<c$91>", "<c$92>", "<c$93>", "<c$94>", "<c$95>", "<c$96>", "<c$97>", "<c$98>", "<c$99>", "<c$9a>", "<c$9b>", "<c$9c>", "<c$9d>", "<c$9e>", "<c$9f>",
        "<c$a0>", "<c$a1>", "<c$a2>", "<c$a3>", "<c$a4>", "<c$a5>", "~", "<c$a7>", "<c$a8>", "<c$a9>", "<c$aa>", "<c$ab>", "<c$ac>", "<c$ad>", "<c$ae>", "<c$af>",
        "<c$b0>", "<c$b1>", "<c$b2>", "<c$b3>", "<c$b4>", "<c$b5>", "<c$b6>", "<c$b7>", "<c$b8>", "<c$b9>", "<c$ba>", "<c$bb>", "<c$bc>", "<c$bd>", "<c$be>", "<c$bf>",
        
//        "<c$c0>", "<c$c1>", "<c$c2>", "<c$c3>", "<c$c4>", "<c$c5>", "<c$c6>", "<c$c7>", "<c$c8>", "<c$c9>", "<c$ca>", "<c$cb>", "<c$cc>", "<c$cd>", "<c$ce>", "<c$cf>",
//     "{SHIFT-*>",      "A",      "B",      "C",      "D",      "E",      "F",      "G",      "H",      "I",      "J",      "K",      "L",      "M",      "N",      "O",
     "\u2014",      "A",      "B",      "C",      "D",      "E",      "F",      "G",      "H",      "I",      "J",      "K",      "L",      "M",      "N",      "O",
     
//        "<c$d0>", "<c$d1>", "<c$d2>", "<c$d3>", "<c$d4>", "<c$d5>", "<c$d6>", "<c$d7>", "<c$d8>", "<c$d9>", "<c$da>", "<c$db>", "<c$dc>", "<c$dd>", "<c$de>", "<c$df>",
//             "P",      "Q",      "R",      "S",      "T",      "U",      "V",      "W",      "X",      "Y",      "Z", "{SHIFT-+>", "<c$DC>", "{SHIFT-->", "<c$DE>", "<c$DF>",
             "P",      "Q",      "R",      "S",      "T",      "U",      "V",      "W",      "X",      "Y",      "Z", "\u2020", "<c$DC>", "\u007c", "<c$DE>", "<c$DF>",

        "<c$e0>", "<c$e1>", "<c$e2>", "<c$e3>", "<c$e4>", "<c$e5>", "<c$e6>", "<c$e7>", "<c$e8>", "<c$e9>", "<c$ea>", "<c$eb>", "<c$ec>", "<c$ed>", "<c$ee>", "<c$ef>",
        "<c$f0>", "<c$f1>", "<c$f2>", "<c$f3>", "<c$f4>", "<c$f5>", "<c$f6>", "<c$f7>", "<c$f8>", "<c$f9>", "<c$fa>", "<c$fb>", "<c$fc>", "<c$fd>", "<c$fe>", "<c$ff>"
        };

        try {
            bufferedWriter.write(petStr[value & 0xff]);
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        return 0;
    }

    private static int outputHelpChar(RandomAccessFile outputStream, char thischar) {
        int charValue = (int)thischar;
        try {
            if((charValue > 64) && (charValue < 91)) {  
                charValue += 0x80;
            } else if((charValue > 96) && (charValue < 123)) {  
                charValue -= 0x20;
            } 
//             else if((charValue > 129) && (charValue < 219)) {  
//                 charValue -= 0xa0;
//             }

            if(charValue == 8212) {
                outputStream.writeByte(0xc0);  // horizontal line
            } else if(charValue == 8224) {
//                outputStream.writeByte(123);   // line cross
                outputStream.writeByte(219);   // line cross
            } else if(charValue == 0x7c) {
                outputStream.writeByte(221);    // vertical line
            } else if(charValue == 0x7e) {
//                outputStream.writeByte(222);    // checkerboard / pi
//                outputStream.writeByte(255);    // checkerboard / pi
                outputStream.writeByte(166);    // checkerboard
            } else if(charValue > 255) {
                System.out.println("WARNING: untranslated UTF8 char: "+thischar+ " "+(int)thischar);
                outputStream.writeByte(thischar);
            } else {
                outputStream.writeByte(charValue);
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        return 0;
    }

    public static void toText(String inputFile, String outputFile) {
        long[] posList = new long[MAXPAGES]; 
        try (
            RandomAccessFile inputStream = new RandomAccessFile(new File(inputFile),"r");

            FileOutputStream outStream = new FileOutputStream(outputFile);
            OutputStreamWriter writer = new OutputStreamWriter(outStream, "UTF-8");
            BufferedWriter outputStream = new BufferedWriter(writer);

        ) {
            byte[] shortCutBlock = new byte[5];
            int iPagNum = 0;
            try {
                do {
                    long pos = inputStream.getFilePointer();
                    posList[iPagNum/10] = pos;iPagNum+=10;
                    while (inputStream.read(shortCutBlock) >= 0) {
                        if (shortCutBlock[0] == 0) break;
                    }
                    while (inputStream.readByte() != 0) {};
                } while(true);
            } catch (EOFException ex) {
            }
            inputStream.seek(0);
            int iPagNum2 = 0;
            long filesize = inputStream.length();
            try {
                do {
                    String headText = String.format("\r<page %04d>\r",iPagNum2);iPagNum2+=10;
                    outputStream.write(headText);
                    while (inputStream.read(shortCutBlock) >= 0) {
                        byte shortCut = shortCutBlock[0];
                        if (shortCut == 0) break;
                        long offSet = getOffset(shortCutBlock);
                        int pagNumLink = getLink(posList,offSet,iPagNum/10) * 10;
                        outputStream.write("<");
                        if (shortCut>=65 && shortCut<=90) {
                            outputPetsciiByte(outputStream, shortCutBlock[0]);
                        }   else {
                            String shortText = String.format("$%02X",((int)shortCut) & 0xFF);
                            outputStream.write(shortText);
                        }
                        String linkText = String.format(" %04d",pagNumLink);
                        outputStream.write(linkText);
                        outputStream.write(">");
                    }
                    outputStream.write("\n");
                    byte inByte;
                    int iCharCount = 0;
                    while ((inByte = inputStream.readByte()) != 0) {
                        int inInt = ((int)inByte) & 0xFF;

                        if ((inInt != 13)) {
                            outputPetsciiByte(outputStream, inByte);
                        } else {
                            outputStream.write("\n");
                        }

                        if (inInt == 13) {
                            iCharCount = 0;
                        } else if (inInt > 9) {
                            iCharCount++;
                        }
                        // When we have a full screen line add a CR
                        if (iCharCount == 40) {
                            iCharCount = 0;
                            outputStream.write("\n");
                        }

                    }
                } while(inputStream.getFilePointer()<filesize);
            } catch (EOFException ex) {
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    public static void toHelp(String inputFile, String outputFile) {
        long[] posList = new long[MAXPAGES];
        int iPagNum = 0;
        byte[] shortCutBlock = new byte[5];
        try {
            FileInputStream inputStream = new FileInputStream(inputFile);
            InputStreamReader reader = new InputStreamReader(inputStream, "UTF-8");
            BufferedReader bufferedReader = new BufferedReader(reader);
            RandomAccessFile outputStream = new RandomAccessFile(new File(outputFile),"rw");

            String line = null;
            int iMode = 0;
            int pageIndex;
            boolean lastLineFull = false;
            while ((line = bufferedReader.readLine()) != null) {
                if (line.startsWith(utf8BOM)) {
                    line = line.substring(1);
                }
                switch (iMode) {
                    case 0:
                        // find start of page, if found get page number and go to mode 1
                        pageIndex = line.indexOf("<page ");
                        if (pageIndex >= 0) {
                            String pagNumStr = line.substring(pageIndex+6,pageIndex+10);
                            iPagNum = Integer.parseInt(pagNumStr);
                            if (posList[iPagNum] != 0) {
                                System.out.println("Double page number defined: "+iPagNum);
                                System.exit(-1);
                            }
                            posList[iPagNum] = outputStream.getFilePointer(); 
                            iMode = 1;
                        }
                        break;
                    case 1:
                        int linkIndex = line.indexOf("<");
                        if (linkIndex >= 0) {
                            do {
                                byte hotKey = 0;
                                if (line.charAt(linkIndex+1) == '$') {
                                    int hexNum = Integer.parseInt(line.substring(linkIndex+2,linkIndex+4),16);
                                    hotKey = (byte)hexNum;
                                    linkIndex+=5;
                                } else {
                                    hotKey = (byte)line.charAt(linkIndex+1);
                                    // convert ascii->petscii
                                    if((hotKey > 64) && (hotKey < 91)) {
                                        hotKey += 0x80;
                                    } else if((hotKey > 96) && (hotKey < 123)) {  
                                        hotKey -= 0x20;
                                    } 
                                    linkIndex+=3;
                                }
                                int linkPage = Integer.parseInt(line.substring(linkIndex,linkIndex+4),10);
                                shortCutBlock[0] = hotKey;
                                shortCutBlock[1] = (byte)(linkPage & 0xff);
                                shortCutBlock[2] = (byte)((linkPage >> 8) & 0xff);
                                shortCutBlock[3] = 0;
                                shortCutBlock[4] = 0;
                                outputStream.write(shortCutBlock);
                                linkIndex = line.indexOf("<",linkIndex+4);
                            } while (linkIndex > 0);
                            Arrays.fill( shortCutBlock, (byte)0);
                            outputStream.write(shortCutBlock); 
                            iMode = 2;
                        }
                        break;
                    case 2:
                        pageIndex = line.indexOf("<page ");
                        if (pageIndex >= 0) {
                            // Remove CR at the end of the previous page
                            if (!lastLineFull) {
                                outputStream.seek(outputStream.getFilePointer()-1);
                            }
                            outputStream.writeByte(0);
                            String pagNumStr = line.substring(pageIndex+6,pageIndex+10);
                            iPagNum = Integer.parseInt(pagNumStr);
                            posList[iPagNum] = outputStream.getFilePointer(); 
                            iMode = 1;
                        } else {
                            while ((pageIndex = line.indexOf("<c$")) >= 0) {
                                int color = Integer.parseInt(line.substring(pageIndex + 3,pageIndex + 5),16); 
                                char[] chArray = new char[1];
                                chArray[0]=(char)color;
                                line = line.substring(0,pageIndex) + new String(chArray) + line.substring(pageIndex+6);
                            }

                            for(int i=0;i<line.length();i++) {
                                outputHelpChar(outputStream, line.charAt(i));
                            }

                            // Check linelength without control characters
                            int lineLength = 0;
                            for(int i=0;i<line.length();i++) {
                                if(line.charAt(i) > 9) {
                                    lineLength++;
                                }
                            }
                            // Add CR only when screen line not full
                            if (lineLength != 40) {
                                outputStream.writeByte(13);
                                lastLineFull = false;
                            } else {
                                lastLineFull = true;
                            }
                        }
                        break;
                }
            }
            outputStream.writeByte(0);
            outputStream.seek(0);
            try {
                do {
                    while (outputStream.read(shortCutBlock) >= 0) {
                        if (shortCutBlock[0] == 0) break;
                        int iPage = getPageNum(shortCutBlock);
                        long lOffset = posList[iPage];
                        shortCutBlock[1] = (byte)(lOffset & 0xff);
                        shortCutBlock[2] = (byte)((lOffset >> 8) & 0xff);
                        shortCutBlock[3] = (byte)((lOffset >> 16) & 0xff);
                        outputStream.seek(outputStream.getFilePointer()-5);
                        outputStream.write(shortCutBlock);
                    }
                    while (outputStream.readByte() != 0) {};
                } while(true);
            } catch (EOFException ex) {
            }
            reader.close();
            outputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
          System.out.println("SYNTAX ERROR on File page:"+iPagNum);
        }
    }

    public static void main(String[] args) {
        if (args.length < 3) {
            System.out.println("Usage: ChamHelp -d/e infile outfile (encode/decode)\n");
            System.exit(0);
        }
 
        String command = args[0];
        if (command.equals("-d")) { 
            toText(args[1],args[2]);
        } else if (command.equals("-e")) {
            toHelp(args[1],args[2]);
        }

    }
}
