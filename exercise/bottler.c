int main(){
	short step = 0;
	short actuators = 0;
	while(!stopped){
		int sensors = readSensorState();
		switch(step){
			case 0:
				step0();
				if(condition)
					step = 1;
				break;
			case 1:
				step1();
				if(condition)
					step = 2;
				break;
			case 2:
				step2();
				if(condition)
					step = 3;
				break;
			case 3:
				step3();
				if(condition)
					step = 4;
				break;
			case 4:
				step4();
				if(condition)
					step = 5;
				break;
			case 5:
				step5();
				if(condition)
					step = 0;
				break;
		}
	}
	return 0;
}
