function Convert-Temperature {
    param(
        [string]$TemperatureInput
    )
    
    $kelvinPattern = '^(-?\d+\.?\d*)\s*[Kk]$'
    $fahrenheitPattern = '^(-?\d+\.?\d*)\s*[Ff]$'
    $celsiusPattern = '^(-?\d+\.?\d*)\s*[Cc]$'
    $numberPattern = '^(-?\d+\.?\d*)$'
    
    $value = 0
    $sourceUnit = ""
    
    if ($TemperatureInput -match $kelvinPattern) {
        $value = [double]$matches[1]
        $sourceUnit = "K"
        Write-Host "Opredelena temperatura v Kelvinah: $value K" -ForegroundColor Cyan
    }
    elseif ($TemperatureInput -match $fahrenheitPattern) {
        $value = [double]$matches[1]
        $sourceUnit = "F"
        Write-Host "Opredelena temperatura v Farengeitah: $value °F" -ForegroundColor Cyan
    }
    elseif ($TemperatureInput -match $celsiusPattern) {
        $value = [double]$matches[1]
        $sourceUnit = "C"
        Write-Host "Opredelena temperatura v Celsiyah: $value °C" -ForegroundColor Cyan
    }
    elseif ($TemperatureInput -match $numberPattern) {
        $value = [double]$matches[1]
        $sourceUnit = "C"
        Write-Host "Opredelena temperatura (po umolchaniyu Celsiy): $value °C" -ForegroundColor Cyan
    }
    else {
        Write-Host "Error: neverny format vvoda!" -ForegroundColor Red
        Write-Host "Ispolzuyte format: chislo[K|C|F] ili prosto chislo (po umolchaniyu °C)" -ForegroundColor Yellow
        Write-Host "Primery: 100K, 25C, 77F, 20" -ForegroundColor Yellow
        return
    }
    
    if ($sourceUnit -eq "K" -and $value -lt 0) {
        Write-Host "Error: temperatura v Kelvinah ne mozhet byt nizhe absolyutnogo nulya (0K)!" -ForegroundColor Red
        return
    }
    
    switch ($sourceUnit) {
        "C" {
            $celsius = $value
            $fahrenheit = ($celsius * 9/5) + 32
            $kelvin = $celsius + 273.15
        }
        "F" {
            $fahrenheit = $value
            $celsius = ($fahrenheit - 32) * 5/9
            $kelvin = ($fahrenheit - 32) * 5/9 + 273.15
        }
        "K" {
            $kelvin = $value
            $celsius = $kelvin - 273.15
            $fahrenheit = ($kelvin - 273.15) * 9/5 + 32
        }
    }
    
    Write-Host "`n=== REZULTATY KONVERTACII ===" -ForegroundColor Green
    Write-Host "Celsiy:    $([Math]::Round($celsius, 2)) °C" -ForegroundColor Yellow
    Write-Host "Farengeit: $([Math]::Round($fahrenheit, 2)) °F" -ForegroundColor Yellow
    Write-Host "Kelvin:    $([Math]::Round($kelvin, 2)) K" -ForegroundColor Yellow
}

function Interactive-TemperatureConverter {
    Write-Host "=== KONVERTER TEMPERATURY ===" -ForegroundColor Green
    Write-Host "Podderzhivaemye formaty vvoda:" -ForegroundColor Yellow
    Write-Host "100K  - Kelviny" -ForegroundColor White
    Write-Host "25C   - Celsii" -ForegroundColor White
    Write-Host "77F   - Farengeity" -ForegroundColor White
    Write-Host "20    - Celsii (po umolchaniyu)" -ForegroundColor White
    Write-Host "quit  - Vyhod" -ForegroundColor Red
    Write-Host "==========================" -ForegroundColor Green
    
    do {
        $input = Read-Host "`nVvedite temperaturu dlya konvertacii"
        
        if ($input -eq "quit" -or $input -eq "exit") {
            Write-Host "Poka Poka!" -ForegroundColor Yellow
            break
        }
        
        if (-not [string]::IsNullOrWhiteSpace($input)) {
            Convert-Temperature -TemperatureInput $input
        }
    } while ($true)
}

Interactive-TemperatureConverter