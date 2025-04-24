## As some user on multipass host OS:

### Create, provision multipass VM named magaox01, plus second cloned VM named magaox02

* N.B. Roles will change to magaox01 and magaox02 after provisioning
```
./magaox_multipass_setup.sh -v=magaox01 -2=magaox02 -M=drbitboy/MagAOX,MagAOX,resurrector-20250424 -r=drbitboy/magao-x-config,config,resurrector-indi-compression
```

## As user ubuntu on both magaox01 and magaox02:

### Cache config file that user xsup can use; switch to user xsup

```
resuctrl reset   ### Creation, provisioning are complete after this step
sudo su - xsup
```

## As user xsup:

### Start INDI server and all INDI drivers under resurrector control
### Update maths_1 and maths_2 INDI drivers, view results

```
resuctrl startup
resuctrl status --all

setINDI maths_1.val.value=1.1   ### on magaox01
setINDI maths_2.val.value=1.2   ### on magaox02

getINDI -t 1
```

### Stop an INDI driver

```
resuctrl stop maths_1           ### on magaox01
resuctrl status                 ### --all is implied
```

### Start an INDI driver

```
resuctrl start maths_1          ### on magaox01
resuctrl status                 ### --all is implied
```

### Stop everything

```
resuctrl stop --all
```
